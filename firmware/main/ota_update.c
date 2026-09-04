#include "ota_update.h"

#include "app_config.h"

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <stdint.h>
#include <string.h>

#include "cJSON.h"

#ifndef HOST_TEST
#include "mqtt_ha.h"

#include "esp_crt_bundle.h"
#include "esp_http_client.h"
#include "esp_https_ota.h"
#include "esp_log.h"
#include "esp_ota_ops.h"
#include "esp_partition.h"
#include "esp_system.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "mbedtls/sha256.h"

static const char *TAG = "ota";
#endif

bool ota_semver_parse(const char *s, int *maj, int *min, int *pat)
{
    if (!s || !maj || !min || !pat) {
        return false;
    }
    const char *p = s;
    char *end = NULL;
    long a = strtol(p, &end, 10);
    if (end == p || *end != '.' || a < 0) {
        return false;
    }
    p = end + 1;
    long b = strtol(p, &end, 10);
    if (end == p || *end != '.' || b < 0) {
        return false;
    }
    p = end + 1;
    long c = strtol(p, &end, 10);
    if (end == p || *end != '\0' || c < 0) {
        return false;
    }
    *maj = (int)a;
    *min = (int)b;
    *pat = (int)c;
    return true;
}

int ota_semver_cmp(const char *a, const char *b)
{
    int am, an, ap, bm, bn, bp;
    if (!ota_semver_parse(a, &am, &an, &ap) || !ota_semver_parse(b, &bm, &bn, &bp)) {
        return 0; /* unparsable → not-newer */
    }
    if (am != bm) {
        return am - bm;
    }
    if (an != bn) {
        return an - bn;
    }
    return ap - bp;
}

bool ota_sha256_hex_valid(const char *hex)
{
    if (!hex || strlen(hex) != OTA_SHA256_HEX_LEN) {
        return false;
    }
    for (size_t i = 0; i < OTA_SHA256_HEX_LEN; i++) {
        char c = hex[i];
        if (!((c >= '0' && c <= '9') || (c >= 'a' && c <= 'f'))) {
            return false;
        }
    }
    return true;
}

static bool copy_json_string(cJSON *obj, const char *key, char *dst, size_t cap, bool required)
{
    cJSON *n = cJSON_GetObjectItemCaseSensitive(obj, key);
    if (!cJSON_IsString(n) || !n->valuestring) {
        return !required;
    }
    if (strlen(n->valuestring) >= cap) {
        return false;
    }
    strncpy(dst, n->valuestring, cap - 1);
    dst[cap - 1] = '\0';
    return true;
}

bool ota_manifest_parse(const char *json, const char *expected_firmware_id, ota_manifest_t *out)
{
    if (!json || !expected_firmware_id || !out) {
        return false;
    }
    memset(out, 0, sizeof(*out));

    cJSON *root = cJSON_Parse(json);
    if (!root || !cJSON_IsObject(root)) {
        cJSON_Delete(root);
        return false;
    }

    cJSON *mv = cJSON_GetObjectItemCaseSensitive(root, "manifest_version");
    if (!cJSON_IsNumber(mv) || mv->valuedouble != 1.0) {
        cJSON_Delete(root);
        return false;
    }
    out->manifest_version = 1;

    if (!copy_json_string(root, "firmware_id", out->firmware_id, sizeof(out->firmware_id), true) ||
        strcmp(out->firmware_id, expected_firmware_id) != 0) {
        cJSON_Delete(root);
        return false;
    }
    if (!copy_json_string(root, "version", out->version, sizeof(out->version), true)) {
        cJSON_Delete(root);
        return false;
    }
    {
        int maj, min, pat;
        if (!ota_semver_parse(out->version, &maj, &min, &pat)) {
            cJSON_Delete(root);
            return false;
        }
    }
    if (!copy_json_string(root, "url", out->url, sizeof(out->url), true)) {
        cJSON_Delete(root);
        return false;
    }
    if (strncmp(out->url, "https://", 8) != 0) {
        cJSON_Delete(root);
        return false;
    }

    cJSON *sha = cJSON_GetObjectItemCaseSensitive(root, "sha256");
    if (sha) {
        if (!cJSON_IsString(sha) || !sha->valuestring || !ota_sha256_hex_valid(sha->valuestring)) {
            cJSON_Delete(root);
            return false;
        }
        out->has_sha256 = true;
        memcpy(out->sha256, sha->valuestring, OTA_SHA256_HEX_LEN + 1);
    }

    cJSON *sz = cJSON_GetObjectItemCaseSensitive(root, "size");
    if (sz) {
        if (!cJSON_IsNumber(sz) || sz->valuedouble < 0) {
            cJSON_Delete(root);
            return false;
        }
        out->has_size = true;
        out->size = (int64_t)sz->valuedouble;
    }

    (void)copy_json_string(root, "release_url", out->release_url, sizeof(out->release_url), false);
    (void)copy_json_string(root, "title", out->title, sizeof(out->title), false);
    (void)copy_json_string(root, "summary", out->summary, sizeof(out->summary), false);

    cJSON_Delete(root);
    return true;
}

bool ota_manifest_is_newer(const ota_manifest_t *m, const char *installed_version)
{
    if (!m || !installed_version) {
        return false;
    }
    return ota_semver_cmp(m->version, installed_version) > 0;
}

bool ota_should_poll_manifest(bool softap_active, bool ota_in_flight, bool mqtt_session_ready,
                              bool mqtt_ready_trigger, uint32_t now_ms, uint32_t last_poll_ms,
                              uint32_t poll_interval_ms, uint32_t min_interval_ms)
{
    if (softap_active || ota_in_flight || !mqtt_session_ready) {
        return false;
    }
    uint32_t since = now_ms - last_poll_ms;
    if (last_poll_ms != 0 && since < min_interval_ms) {
        return false;
    }
    if (mqtt_ready_trigger) {
        return true;
    }
    if (last_poll_ms == 0) {
        return true;
    }
    return since >= poll_interval_ms;
}

bool ota_should_start_install(const char *installed_version, const char *latest_version,
                              bool has_cached_url, bool softap_active, bool already_in_progress)
{
    if (softap_active || already_in_progress || !has_cached_url) {
        return false;
    }
    if (!installed_version || !latest_version || !latest_version[0]) {
        return false;
    }
    return ota_semver_cmp(latest_version, installed_version) > 0;
}

bool ota_should_confirm(bool pending_verify, bool mqtt_session_ready)
{
    return pending_verify && mqtt_session_ready;
}

bool ota_confirm_timeout_elapsed(bool pending_verify, uint32_t now_ms, uint32_t boot_ms,
                                 uint32_t timeout_ms)
{
    if (!pending_verify) {
        return false;
    }
    return (now_ms - boot_ms) >= timeout_ms;
}

bool ota_size_fits_slot(bool has_size, int64_t size, size_t slot_bytes)
{
    if (!has_size) {
        return true;
    }
    if (size < 0) {
        return false;
    }
    return (uint64_t)size <= (uint64_t)slot_bytes;
}

int ota_json_escape(char *dst, size_t cap, const char *src)
{
    if (!dst || !src || cap == 0) {
        return -1;
    }
    size_t j = 0;
    for (size_t i = 0; src[i] != '\0'; i++) {
        unsigned char c = (unsigned char)src[i];
        const char *rep = NULL;
        char tmp[2];
        if (c == '"' || c == '\\') {
            if (j + 2 >= cap) {
                return -1;
            }
            dst[j++] = '\\';
            dst[j++] = (char)c;
            continue;
        }
        if (c == '\n') {
            rep = "\\n";
        } else if (c == '\r') {
            rep = "\\r";
        } else if (c == '\t') {
            rep = "\\t";
        } else if (c < 0x20) {
            return -1;
        } else {
            tmp[0] = (char)c;
            tmp[1] = '\0';
            rep = tmp;
        }
        size_t rl = strlen(rep);
        if (j + rl >= cap) {
            return -1;
        }
        memcpy(dst + j, rep, rl);
        j += rl;
    }
    if (j >= cap) {
        return -1;
    }
    dst[j] = '\0';
    return (int)j;
}

static int state_json_append(char *buf, size_t cap, int n, const char *fmt, ...)
{
    if (n < 0 || (size_t)n >= cap) {
        return -1;
    }
    va_list ap;
    va_start(ap, fmt);
    int m = vsnprintf(buf + n, cap - (size_t)n, fmt, ap);
    va_end(ap);
    if (m < 0 || (size_t)n + (size_t)m >= cap) {
        return -1;
    }
    return n + m;
}

static int state_json_append_escaped(char *buf, size_t cap, int n, const char *key, const char *val)
{
    char esc[OTA_MANIFEST_URL_MAX * 2];
    if (ota_json_escape(esc, sizeof(esc), val ? val : "") < 0) {
        return -1;
    }
    return state_json_append(buf, cap, n, ",\"%s\":\"%s\"", key, esc);
}

int ota_build_state_json(char *buf, size_t cap, const ota_progress_state_t *st)
{
    if (!buf || !st || cap < 32) {
        return -1;
    }
    char inst_esc[OTA_VERSION_MAX * 2];
    char lat_esc[OTA_VERSION_MAX * 2];
    const char *lat = st->latest_version[0] ? st->latest_version : st->installed_version;
    if (ota_json_escape(inst_esc, sizeof(inst_esc), st->installed_version) < 0 ||
        ota_json_escape(lat_esc, sizeof(lat_esc), lat) < 0) {
        return -1;
    }
    int n = snprintf(buf, cap,
                     "{\"installed_version\":\"%s\",\"latest_version\":\"%s\","
                     "\"in_progress\":%s",
                     inst_esc, lat_esc, st->in_progress ? "true" : "false");
    if (n < 0 || (size_t)n >= cap) {
        return -1;
    }
    if (st->has_percentage) {
        n = state_json_append(buf, cap, n, ",\"update_percentage\":%d", st->update_percentage);
        if (n < 0) {
            return -1;
        }
    }
    if (st->failed) {
        n = state_json_append(buf, cap, n, ",\"failed\":true");
        if (n < 0) {
            return -1;
        }
    }
    if (st->release_url[0]) {
        n = state_json_append_escaped(buf, cap, n, "release_url", st->release_url);
        if (n < 0) {
            return -1;
        }
    }
    if (st->title[0]) {
        n = state_json_append_escaped(buf, cap, n, "title", st->title);
        if (n < 0) {
            return -1;
        }
    }
    if (st->summary[0]) {
        n = state_json_append_escaped(buf, cap, n, "release_summary", st->summary);
        if (n < 0) {
            return -1;
        }
    }
    if ((size_t)n + 2 > cap) {
        return -1;
    }
    buf[n++] = '}';
    buf[n] = '\0';
    return n;
}

void ota_progress_clear_in_flight(ota_progress_state_t *st)
{
    if (!st) {
        return;
    }
    st->in_progress = false;
    st->has_percentage = false;
    st->failed = false;
}

bool ota_update_is_install_topic(const char *topic, const char *device_id)
{
    if (!topic || !device_id) {
        return false;
    }
    char expect[96];
    snprintf(expect, sizeof(expect), "%s%s/update/set", APP_MQTT_TOPIC_ROOT, device_id);
    return strcmp(topic, expect) == 0;
}

#ifndef HOST_TEST

static char s_device_id[16];
static bool s_softap_active;
static bool s_in_progress;
static bool s_cancel;
static bool s_failed;
static bool s_has_cache;
static bool s_has_percentage;
static int s_percentage;
static uint32_t s_last_poll_ms;
static uint32_t s_boot_ms;
static bool s_boot_ms_set;
static ota_manifest_t s_cache;
static TaskHandle_t s_ota_task;
static TaskHandle_t s_poll_task;
static bool s_poll_inflight;
static bool s_poll_cancel;
static int s_last_pub_pct = -1;
static uint32_t s_last_pub_ms;

#define OTA_MANIFEST_BODY_MAX 65536
#define OTA_PROGRESS_PUB_MIN_MS 1000

static void fill_progress(ota_progress_state_t *st)
{
    memset(st, 0, sizeof(*st));
    strncpy(st->installed_version, APP_FW_VERSION, sizeof(st->installed_version) - 1);
    if (s_has_cache) {
        strncpy(st->latest_version, s_cache.version, sizeof(st->latest_version) - 1);
        strncpy(st->release_url, s_cache.release_url, sizeof(st->release_url) - 1);
        strncpy(st->title, s_cache.title, sizeof(st->title) - 1);
        strncpy(st->summary, s_cache.summary, sizeof(st->summary) - 1);
    } else {
        strncpy(st->latest_version, APP_FW_VERSION, sizeof(st->latest_version) - 1);
    }
    st->in_progress = s_in_progress;
    st->has_percentage = s_has_percentage;
    st->update_percentage = s_percentage;
    st->failed = s_failed;
}

void ota_update_publish_state(void)
{
    if (!s_device_id[0]) {
        return;
    }
    ota_progress_state_t st;
    fill_progress(&st);
    char json[768];
    if (ota_build_state_json(json, sizeof(json), &st) < 0) {
        return;
    }
    char topic[96];
    mqtt_ha_update_state_topic(topic, sizeof(topic), s_device_id);
    mqtt_ha_publish_update_state(s_device_id, json);
}

static bool image_pending_verify(void)
{
    const esp_partition_t *running = esp_ota_get_running_partition();
    esp_ota_img_states_t state;
    if (!running || esp_ota_get_state_partition(running, &state) != ESP_OK) {
        return false;
    }
    return state == ESP_OTA_IMG_PENDING_VERIFY;
}

static void try_confirm(void)
{
    if (!ota_should_confirm(image_pending_verify(), true)) {
        return;
    }
    esp_err_t err = esp_ota_mark_app_valid_cancel_rollback();
    if (err == ESP_OK) {
        ESP_LOGI(TAG, "marked app valid (MQTT session ready)");
    } else if (err != ESP_ERR_INVALID_STATE) {
        ESP_LOGW(TAG, "mark valid failed: %s", esp_err_to_name(err));
    }
}

static esp_err_t http_get_body(const char *url, char **out_body, int *out_len)
{
    *out_body = NULL;
    *out_len = 0;
    esp_http_client_config_t cfg = {
        .url = url,
        .timeout_ms = 15000,
        .crt_bundle_attach = esp_crt_bundle_attach,
        .keep_alive_enable = true,
    };
    esp_http_client_handle_t client = esp_http_client_init(&cfg);
    if (!client) {
        return ESP_ERR_NO_MEM;
    }
    esp_err_t err = esp_http_client_open(client, 0);
    if (err != ESP_OK) {
        esp_http_client_cleanup(client);
        return err;
    }
    int content_length = esp_http_client_fetch_headers(client);
    int status = esp_http_client_get_status_code(client);
    if (status < 200 || status >= 300) {
        esp_http_client_close(client);
        esp_http_client_cleanup(client);
        return ESP_FAIL;
    }

    char *buf = NULL;
    int total = 0;
    if (content_length >= 0) {
        if (content_length > OTA_MANIFEST_BODY_MAX) {
            esp_http_client_close(client);
            esp_http_client_cleanup(client);
            return ESP_ERR_INVALID_SIZE;
        }
        buf = malloc((size_t)content_length + 1);
        if (!buf) {
            esp_http_client_close(client);
            esp_http_client_cleanup(client);
            return ESP_ERR_NO_MEM;
        }
        while (total < content_length) {
            int r = esp_http_client_read(client, buf + total, content_length - total);
            if (r < 0) {
                free(buf);
                esp_http_client_close(client);
                esp_http_client_cleanup(client);
                return ESP_FAIL;
            }
            if (r == 0) {
                break;
            }
            total += r;
        }
    } else {
        buf = malloc(OTA_MANIFEST_BODY_MAX + 1);
        if (!buf) {
            esp_http_client_close(client);
            esp_http_client_cleanup(client);
            return ESP_ERR_NO_MEM;
        }
        while (total < OTA_MANIFEST_BODY_MAX) {
            int r = esp_http_client_read(client, buf + total, OTA_MANIFEST_BODY_MAX - total);
            if (r < 0) {
                free(buf);
                esp_http_client_close(client);
                esp_http_client_cleanup(client);
                return ESP_FAIL;
            }
            if (r == 0) {
                break;
            }
            total += r;
        }
        if (total >= OTA_MANIFEST_BODY_MAX) {
            char probe;
            int r = esp_http_client_read(client, &probe, 1);
            if (r > 0) {
                free(buf);
                esp_http_client_close(client);
                esp_http_client_cleanup(client);
                return ESP_ERR_INVALID_SIZE;
            }
        }
    }
    buf[total] = '\0';
    esp_http_client_close(client);
    esp_http_client_cleanup(client);
    *out_body = buf;
    *out_len = total;
    return ESP_OK;
}

static void apply_manifest(const ota_manifest_t *m)
{
    s_cache = *m;
    s_has_cache = true;
    s_failed = false;
    ota_update_publish_state();
}

static void poll_manifest_now(uint32_t now_ms)
{
    char *body = NULL;
    int len = 0;
    esp_err_t err = http_get_body(OTA_MANIFEST_URL, &body, &len);
    s_last_poll_ms = now_ms;
    if (s_poll_cancel) {
        free(body);
        return;
    }
    if (err != ESP_OK || !body) {
        ESP_LOGW(TAG, "manifest fetch failed (%s)", esp_err_to_name(err));
        free(body);
        return;
    }
    ota_manifest_t m;
    if (!ota_manifest_parse(body, OTA_FIRMWARE_ID, &m)) {
        ESP_LOGW(TAG, "manifest rejected (keep prior cache)");
        free(body);
        return;
    }
    free(body);
    if (s_poll_cancel) {
        return;
    }
    apply_manifest(&m);
    ESP_LOGI(TAG, "manifest ok version=%s newer=%d", m.version,
             (int)ota_manifest_is_newer(&m, APP_FW_VERSION));
}

static void poll_manifest_task(void *arg)
{
    uint32_t now_ms = (uint32_t)(uintptr_t)arg;
    poll_manifest_now(now_ms);
    s_poll_inflight = false;
    s_poll_cancel = false;
    s_poll_task = NULL;
    vTaskDelete(NULL);
}

/**
 * Non-blocking: schedule one-shot poll task (single-flight).
 * Fail-safe / SoftAP tick must never call http_get_body on their own stacks.
 */
static void schedule_manifest_poll(uint32_t now_ms)
{
    if (s_poll_inflight || s_in_progress) {
        return;
    }
    s_poll_inflight = true;
    s_poll_cancel = false;
    /* Stamp last attempt at schedule time so tick/session cannot race another poll. */
    s_last_poll_ms = now_ms;
    BaseType_t ok = xTaskCreate(poll_manifest_task, "ota_poll", 6144,
                                (void *)(uintptr_t)now_ms, 3, &s_poll_task);
    if (ok != pdPASS) {
        s_poll_inflight = false;
        s_poll_task = NULL;
        ESP_LOGW(TAG, "manifest poll task create failed");
    }
}

static bool verify_partition_sha256(const esp_partition_t *part, size_t img_len, const char *expect_hex)
{
    mbedtls_sha256_context ctx;
    mbedtls_sha256_init(&ctx);
    mbedtls_sha256_starts(&ctx, 0);
    uint8_t chunk[1024];
    size_t off = 0;
    while (off < img_len) {
        size_t n = img_len - off;
        if (n > sizeof(chunk)) {
            n = sizeof(chunk);
        }
        if (esp_partition_read(part, off, chunk, n) != ESP_OK) {
            mbedtls_sha256_free(&ctx);
            return false;
        }
        mbedtls_sha256_update(&ctx, chunk, n);
        off += n;
    }
    uint8_t digest[32];
    mbedtls_sha256_finish(&ctx, digest);
    mbedtls_sha256_free(&ctx);
    char hex[OTA_SHA256_HEX_LEN + 1];
    for (int i = 0; i < 32; i++) {
        sprintf(hex + i * 2, "%02x", digest[i]);
    }
    return strcmp(hex, expect_hex) == 0;
}

static void ota_task(void *arg)
{
    (void)arg;
    s_in_progress = true;
    s_failed = false;
    s_has_percentage = true;
    s_percentage = 0;
    s_cancel = false;
    s_last_pub_pct = -1;
    s_last_pub_ms = 0;
    ota_update_publish_state();

    const esp_partition_t *update_part = esp_ota_get_next_update_partition(NULL);
    if (!update_part) {
        ESP_LOGE(TAG, "no inactive OTA slot");
        goto fail;
    }
    if (!ota_size_fits_slot(s_cache.has_size, s_cache.size, update_part->size)) {
        ESP_LOGE(TAG, "manifest size exceeds slot");
        goto fail;
    }

    esp_http_client_config_t http_cfg = {
        .url = s_cache.url,
        .timeout_ms = 30000,
        .crt_bundle_attach = esp_crt_bundle_attach,
        .keep_alive_enable = true,
    };
    esp_https_ota_config_t ota_cfg = {
        .http_config = &http_cfg,
    };

    esp_https_ota_handle_t handle = NULL;
    esp_err_t err = esp_https_ota_begin(&ota_cfg, &handle);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "ota begin failed: %s", esp_err_to_name(err));
        goto fail;
    }

    while (1) {
        if (s_cancel) {
            esp_https_ota_abort(handle);
            ESP_LOGW(TAG, "OTA cancelled");
            goto fail_abort;
        }
        err = esp_https_ota_perform(handle);
        if (err != ESP_ERR_HTTPS_OTA_IN_PROGRESS) {
            break;
        }
        int total = esp_https_ota_get_image_size(handle);
        int read = esp_https_ota_get_image_len_read(handle);
        if (total > 0) {
            int pct = (int)((read * 100LL) / total);
            if (pct > 100) {
                pct = 100;
            }
            s_has_percentage = true;
            s_percentage = pct;
            uint32_t now = xTaskGetTickCount() * portTICK_PERIOD_MS;
            if (pct != s_last_pub_pct || (now - s_last_pub_ms) >= OTA_PROGRESS_PUB_MIN_MS) {
                s_last_pub_pct = pct;
                s_last_pub_ms = now;
                ota_update_publish_state();
            }
        }
        vTaskDelay(pdMS_TO_TICKS(50));
    }

    if (err != ESP_OK) {
        ESP_LOGE(TAG, "ota perform failed: %s", esp_err_to_name(err));
        esp_https_ota_abort(handle);
        goto fail_abort;
    }

    if (!esp_https_ota_is_complete_data_received(handle)) {
        ESP_LOGE(TAG, "incomplete image");
        esp_https_ota_abort(handle);
        goto fail_abort;
    }

    int img_len = esp_https_ota_get_image_len_read(handle);
    if (s_cache.has_sha256) {
        if (img_len <= 0 || !verify_partition_sha256(update_part, (size_t)img_len, s_cache.sha256)) {
            ESP_LOGE(TAG, "sha256 mismatch");
            esp_https_ota_abort(handle);
            goto fail_abort;
        }
    }

    err = esp_https_ota_finish(handle);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "ota finish failed: %s", esp_err_to_name(err));
        goto fail;
    }

    /* Clear in_progress before reboot so retained MQTT is not stuck true. */
    s_in_progress = false;
    s_has_percentage = false;
    s_percentage = 100;
    s_failed = false;
    {
        ota_progress_state_t st;
        fill_progress(&st);
        ota_progress_clear_in_flight(&st);
        char json[768];
        if (ota_build_state_json(json, sizeof(json), &st) >= 0) {
            mqtt_ha_publish_update_state(s_device_id, json);
        }
    }
    ESP_LOGI(TAG, "OTA success — restarting");
    vTaskDelay(pdMS_TO_TICKS(500));
    esp_restart();

fail_abort:
fail:
    s_in_progress = false;
    s_has_percentage = false;
    s_failed = true;
    ota_update_publish_state();
    s_ota_task = NULL;
    vTaskDelete(NULL);
}

esp_err_t ota_update_init(const char *device_id)
{
    if (!device_id || !device_id[0]) {
        return ESP_ERR_INVALID_ARG;
    }
    strncpy(s_device_id, device_id, sizeof(s_device_id) - 1);
    s_softap_active = false;
    s_boot_ms = 0;
    s_boot_ms_set = false;
    ESP_LOGI(TAG, "init manifest=%s", OTA_MANIFEST_URL);
    return ESP_OK;
}

void ota_update_set_softap_active(bool active)
{
    s_softap_active = active;
}

void ota_update_on_mqtt_session_ready(void)
{
    uint32_t now = xTaskGetTickCount() * portTICK_PERIOD_MS;
    if (!s_boot_ms_set) {
        s_boot_ms = now;
        s_boot_ms_set = true;
    }
    try_confirm();
    /* Publish immediately so reboot-gap retained in_progress:true is overwritten
     * before a slow manifest poll returns. */
    ota_update_publish_state();
    if (ota_should_poll_manifest(s_softap_active, s_in_progress || s_poll_inflight, true, true, now,
                                 s_last_poll_ms,
                                 (uint32_t)OTA_MANIFEST_POLL_INTERVAL_S * 1000u,
                                 (uint32_t)OTA_MANIFEST_MIN_INTERVAL_S * 1000u)) {
        schedule_manifest_poll(now);
    }
}

void ota_update_tick(uint32_t now_ms, bool mqtt_session_ready)
{
    /* Confirm timeout + schedule-only poll — never HTTP on this (fail-safe) stack. */
    if (!s_boot_ms_set) {
        s_boot_ms = now_ms;
        s_boot_ms_set = true;
    }
    if (ota_confirm_timeout_elapsed(image_pending_verify(), now_ms, s_boot_ms,
                                    OTA_CONFIRM_TIMEOUT_MS)) {
        ESP_LOGE(TAG, "confirm timeout (%u ms) — restart for rollback",
                 (unsigned)OTA_CONFIRM_TIMEOUT_MS);
        esp_restart();
    }
    if (ota_should_poll_manifest(s_softap_active, s_in_progress || s_poll_inflight, mqtt_session_ready,
                                 false, now_ms, s_last_poll_ms,
                                 (uint32_t)OTA_MANIFEST_POLL_INTERVAL_S * 1000u,
                                 (uint32_t)OTA_MANIFEST_MIN_INTERVAL_S * 1000u)) {
        schedule_manifest_poll(now_ms);
    }
}

esp_err_t ota_update_handle_install(const char *payload)
{
    if (!payload || strcmp(payload, OTA_PAYLOAD_INSTALL) != 0) {
        return ESP_ERR_INVALID_ARG;
    }
    if (!ota_should_start_install(APP_FW_VERSION, s_has_cache ? s_cache.version : "",
                                  s_has_cache && s_cache.url[0], s_softap_active, s_in_progress)) {
        s_failed = true;
        ota_update_publish_state();
        ESP_LOGW(TAG, "Install rejected");
        return ESP_ERR_INVALID_STATE;
    }
    if (s_ota_task) {
        return ESP_ERR_INVALID_STATE;
    }
    BaseType_t ok = xTaskCreate(ota_task, "ota", 8192, NULL, 3, &s_ota_task);
    return ok == pdPASS ? ESP_OK : ESP_ERR_NO_MEM;
}

bool ota_update_in_progress(void)
{
    return s_in_progress;
}

void ota_update_cancel(void)
{
    s_cancel = true;
    s_poll_cancel = true; /* abandon in-flight manifest apply after HTTP returns */
}

#endif /* !HOST_TEST */
