# Melnyk OpenThermMessageID enum (IDs beyond / clarifying v2.2)

Source: https://github.com/ihormelnyk/opentherm_library `src/OpenTherm.h`
(`OpenThermMessageID`), MIT. Captured for knowledge ingest as the common firmware
catalog of Data IDs 0..127 including post-v2.2 assignments (ventilation / heat-recovery,
solar storage, brand strings, extra sensors).

IDs also present in OpenTherm Protocol v2.2 are omitted here; see
`2003-02-07-opentherm-protocol-v2-2-data-id-directory.md`.

| ID | Enum name | Type (comment) | Description (comment) |
|----|-----------|----------------|------------------------|
| 34 | TboilerHeatExchanger | f8.8 | Boiler heat exchanger temperature (°C) |
| 35 | BoilerFanSpeedSetpointAndActual | u8 / u8 | Boiler fan speed Setpoint and actual value |
| 36 | FlameCurrent | f8.8 | Electrical current through burner flame (μA) |
| 37 | TrCH2 | f8.8 | Room temperature for 2nd CH circuit (°C) |
| 38 | RelativeHumidity | f8.8 | Actual relative humidity as a percentage |
| 39 | TrOverride2 | f8.8 | Remote Override Room Setpoint 2 |
| 70 | StatusVentilationHeatRecovery | flag8 / flag8 | Master and Slave Status flags ventilation / heat-recovery |
| 71 | Vset | - / u8 | Relative ventilation position (0–100%) |
| 72 | ASFflagsOEMfaultCodeVentilationHeatRecovery | flag8 / u8 | Application-specific fault flags and OEM fault code V/HR |
| 73 | OEMDiagnosticCodeVentilationHeatRecovery | u16 | OEM-specific diagnostic/service code V/HR |
| 74 | SConfigSMemberIDCodeVentilationHeatRecovery | flag8 / u8 | Slave Configuration Flags / MemberID Code V/HR |
| 75 | OpenThermVersionVentilationHeatRecovery | f8.8 | Implemented OpenTherm version in V/HR system |
| 76 | VentilationHeatRecoveryVersion | u8 / u8 | Ventilation / heat-recovery product version number and type |
| 77 | RelVentLevel | - / u8 | Relative ventilation (0–100%) |
| 78 | RHexhaust | - / u8 | Relative humidity exhaust air (0–100%) |
| 79 | CO2exhaust | u16 | CO2 level exhaust air (0–2000 ppm) |
| 80 | Tsi | f8.8 | Supply inlet temperature (°C) |
| 81 | Tso | f8.8 | Supply outlet temperature (°C) |
| 82 | Tei | f8.8 | Exhaust inlet temperature (°C) |
| 83 | Teo | f8.8 | Exhaust outlet temperature (°C) |
| 84 | RPMexhaust | u16 | Exhaust fan speed in rpm |
| 85 | RPMsupply | u16 | Supply fan speed in rpm |
| 86 | RBPflagsVentilationHeatRecovery | flag8 / flag8 | Remote V/HR parameter transfer-enable & read/write flags |
| 87 | NominalVentilationValue | u8 / - | Nominal relative value for ventilation (0–100%) |
| 88 | TSPventilationHeatRecovery | u8 / u8 | Number of TSPs supported by V/HR |
| 89 | TSPindexTSPvalueVentilationHeatRecovery | u8 / u8 | Index / value of referred-to V/HR TSP |
| 90 | FHBsizeVentilationHeatRecovery | u8 / u8 | Size of Fault-History-Buffer supported by V/HR |
| 91 | FHBindexFHBvalueVentilationHeatRecovery | u8 / u8 | Index / value of referred-to V/HR FHB entry |
| 93 | Brand | u8 / u8 | Brand text string character index / ASCII character |
| 94 | BrandVersion | u8 / u8 | Brand version text string character index / ASCII character |
| 95 | BrandSerialNumber | u8 / u8 | Brand serial text string character index / ASCII character |
| 96 | CoolingOperationHours | u16 | Hours slave is in Cooling Mode (reset by zero optional) |
| 97 | PowerCycles | u16 | Number of Power Cycles of a slave (reset by zero optional) |
| 98 | RFsensorStatusInformation | special / special | RF strength and battery level for a specific RF sensor |
| 99 | RemoteOverrideOperatingModeHeatingDHW | special / special | Operating Mode HC1, HC2 / Operating Mode DHW |
| 101 | StatusSolarStorage | flag8 / flag8 | Master and Slave Status flags Solar Storage |
| 102 | ASFflagsOEMfaultCodeSolarStorage | flag8 / u8 | Application-specific fault flags and OEM fault code Solar Storage |
| 103 | SConfigSMemberIDcodeSolarStorage | flag8 / u8 | Slave Configuration Flags / MemberID Code Solar Storage |
| 104 | SolarStorageVersion | u8 / u8 | Solar Storage product version number and type |
| 105 | TSPSolarStorage | u8 / u8 | Number of TSPs supported by Solar Storage |
| 106 | TSPindexTSPvalueSolarStorage | u8 / u8 | Index / value of referred-to Solar Storage TSP |
| 107 | FHBsizeSolarStorage | u8 / u8 | Size of Fault-History-Buffer supported by Solar Storage |
| 108 | FHBindexFHBvalueSolarStorage | u8 / u8 | Index / value of referred-to Solar Storage FHB entry |
| 109 | ElectricityProducerStarts | u16 | Number of starts of the electricity producer |
| 110 | ElectricityProducerHours | u16 | Hours the electricity producer is in operation |
| 111 | ElectricityProduction | u16 | Current electricity production in Watt |
| 112 | CumulativElectricityProduction | u16 | Cumulative electricity production in KWh |
| 113 | UnsuccessfulBurnerStarts | u16 | Number of unsuccessful burner starts |
| 114 | FlameSignalTooLowNumber | u16 | Number of times flame signal was too low |

## Type note vs v2.2

Melnyk lists ID 30 `Tcollector` as f8.8; Protocol v2.2 directory lists it as s16. Prefer the protocol for wire encoding when they disagree.
