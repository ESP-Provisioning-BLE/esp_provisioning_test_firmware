# Changelog

All notable changes to this project are documented in this file.

The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.1.0/), and this project follows [Semantic Versioning](https://semver.org/spec/v2.0.0.html) loosely: it is a test artifact rather than a published package, so `minor` marks a new test feature and `patch` a firmware fix.

## [Unreleased]

### Added

- Protocomm Security 0 (plain text, no proof-of-possession) as a third choice of the provisioning security version, to test Security0 in the package ([esp_provisioning_ble#31](https://github.com/ESP-Provisioning-BLE/esp_provisioning_ble/pull/31)). Security 1 stays the default; the `sdkconfig.ci.sec0` override selects Security 0, and CI builds it as a separate variant.

## [0.1.0] - 2026-07-12

### Added

- Initial ESP32-C6 test firmware for exercising the [`esp_provisioning_ble`](https://github.com/ESP-Provisioning-BLE/esp_provisioning_ble) Dart package against real hardware over BLE provisioning, starting from the shape of the ESP-IDF `wifi_prov_mgr` example and pinned to ESP-IDF v5.5.4.
- Forced BLE transport and protocomm Security 1 in `sdkconfig.defaults`, so provisioning works with the package out of the box. The upstream example defaults to Security 2, which the package does not support yet ([esp_provisioning_ble#30](https://github.com/ESP-Provisioning-BLE/esp_provisioning_ble/issues/30)).
- `CONFIG_TESTFW_<NAME>` convention for togglable test behaviours, each documented against the package issue/PR that motivates it.
- `CONFIG_TESTFW_APPLY_BSSID` (default on): honours the BSSID and channel the client sends in the provisioning SetConfig request, which upstream ESP-IDF validates but never applies. Verified on hardware to select a specific access point among two sharing the same SSID. Motivated by [esp_provisioning_ble#15](https://github.com/ESP-Provisioning-BLE/esp_provisioning_ble/pull/15).
- Build-only CI (GitHub Actions) pinned to ESP-IDF v5.5.4 and target esp32c6, building the feature both on and off.

### Security

- The credential-received log prints only the length of the received Wi-Fi password, never the password itself, so it cannot leak into shared logs or screenshots.

[unreleased]: https://github.com/ESP-Provisioning-BLE/esp_provisioning_test_firmware/compare/v0.1.0...HEAD
[0.1.0]: https://github.com/ESP-Provisioning-BLE/esp_provisioning_test_firmware/releases/tag/v0.1.0
