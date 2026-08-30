# OpenTherm

* [Data ID 0 Status](opentherm/data-id-0-status.md) - Mandatory status exchange; master CH/DHW flags and slave fault/activity bits.
* [Data ID 1 TSet](opentherm/data-id-1-tset.md) - Control setpoint (CH water temperature 0..100°C); master WRITE, f8.8 encoded.
* [Data ID 25 Tboiler](opentherm/data-id-25-tboiler.md) - Boiler flow water temperature; slave READ, f8.8 encoded.
* [Melnyk OpenTherm Library](opentherm/melnyk-opentherm-library.md) - OpenTherm v2.2 master library for Arduino/ESP32; interrupt-driven Manchester framing.
* [OpenTherm Data Encoding](opentherm/opentherm-data-encoding.md) - f8.8 temperature encoding and flag8 status bit packing.
* [OpenTherm Data IDs](opentherm/opentherm-data-ids.md) - Full catalog of OpenTherm Data IDs 0–127 (v2.2 directory plus common post-v2.2 assignments).
* [OpenTherm Frame Format](opentherm/opentherm-frame-format.md) - OT/+ 32-bit frame layout, message types, and master/slave timing rules.
* [OpenTherm Protocol](opentherm/opentherm-protocol.md) - Point-to-point master/slave HVAC bus; OT/+ digital and OT/- analogue levels.
* [Sazanof ESP-IDF OpenTherm](opentherm/sazanof-esp-idf-opentherm.md) - Native ESP-IDF OpenTherm master component (sazanof/opentherm); GPIO interrupt + esp_timer framing.
* [xyzroe OpenTherm-TTL Adapter](opentherm/xyzroe-opentherm-ttl-adapter.md) - Opto-isolated OpenTherm↔TTL module (RS485-sized); RX/TX/VCC/GND header.
