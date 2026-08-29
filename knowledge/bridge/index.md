# Bridge

* [Catalog Commands](bridge/catalog-commands.md) - RescanCatalog and ClearCatalog manufacturer commands on the spillover cluster.
* [Discovery Catalog](bridge/discovery-catalog.md) - OpenTherm Data ID discovery, NVS cache, boot validation, and available vs unknown classification.
* [End-to-End Control Flow](bridge/end-to-end-control-flow.md) - Zigbee join, Thermostat callbacks, tiered OpenTherm poll engine, and layered ZCL routing.
* [Endpoint Layout](bridge/endpoint-layout.md) - Zigbee endpoint topology — Thermostat, discovery-driven standard clusters, and spillover.
* [Local Temperature Mapping](bridge/local-temperature-mapping.md) - OpenTherm Data ID 25 Tboiler to Zigbee LocalTemperature reporting via layered routing.
* [OpenTherm GPIO Wiring](bridge/opentherm-gpio-wiring.md) - Default GPIO12/13 pin assignment for OpenTherm adapter on WeAct ESP32-C6-A.
* [Poll Tiers](bridge/poll-tiers.md) - Fast, slow, and promoted poll scheduling with time-budgeted multi-read per tick.
* [Spillover Encoding](bridge/spillover-encoding.md) - Manufacturer cluster 0xFC01 attribute encoding, invalid sentinels, and spillover-only routing.
* [Water Setpoint Mapping](bridge/water-setpoint-mapping.md) - Zigbee OccupiedHeatingSetpoint to OpenTherm Data ID 1 TSet conversion.
