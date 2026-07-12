# esp_provisioning_test_firmware

Reproducible ESP32 firmware used to verify the Flutter/Dart package [`esp_provisioning_ble`](https://github.com/ESP-Provisioning-BLE/esp_provisioning_ble) against real hardware over BLE Wi-Fi provisioning.

> [!IMPORTANT]
> **This is a test target, not production firmware.** It exists to make behaviours of the `esp_provisioning_ble` package easy to reproduce and observe on a real ESP32. It starts from the official ESP-IDF example `examples/provisioning/wifi_prov_mgr` and is customised from there; it is **not** a fork of ESP-IDF and has **not** been reviewed for production use.

## Why this exists

Two problems only show up when testing the Dart package against a real ESP32, and both are annoying to rediscover by hand every time. This firmware bakes in the fixes so you can flash it and go:

1. **Security 2 by default.** The upstream example defaults to protocomm Security 2, which the package does not support yet ([`esp_provisioning_ble#30`](https://github.com/ESP-Provisioning-BLE/esp_provisioning_ble/issues/30)). With Security 2 the handshake never completes and it looks like a package bug. This firmware forces **Security 1** (see `sdkconfig.defaults`), with the proof-of-possession hard-coded to `abcd1234` (the same value the package's example pre-fills).
2. **The BSSID is silently ignored.** ESP-IDF's `wifi_provisioning` component validates the BSSID sent by the client but never applies it to the Wi-Fi STA config, so on stock firmware sending a BSSID does not change which access point the ESP32 joins. This firmware can honour it, behind a toggle (see [Test features](#test-features)). Motivated by [`esp_provisioning_ble#15`](https://github.com/ESP-Provisioning-BLE/esp_provisioning_ble/pull/15).

## Requirements

- **ESP-IDF v5.5.4** (pinned; see [`AGENTS.md`](AGENTS.md) for why this exact version and why it is a single version rather than a matrix).
- An **ESP32-C6** board (a DevKitM-1 is what we test on). Other BLE-capable ESP32s should work; the ESP32-S2 will not (no BLE).

## Quickstart

```bash
. $IDF_PATH/export.sh          # activate ESP-IDF v5.5.4
idf.py set-target esp32c6
idf.py build                   # sdkconfig.defaults is applied automatically
idf.py -p <PORT> flash monitor
```

In the log, look for:

```
I (....) wifi_prov_mgr: Provisioning started with service name : PROV_XXXXXX
```

`PROV_XXXXXX` (last three bytes of the board MAC) is the BLE name to scan for from the phone. Provision it with the package's example app; the proof-of-possession is `abcd1234`.

Once provisioned, the device stops advertising on the next boot. To test again from scratch: `idf.py -p <PORT> erase-flash` and re-flash, or build with `CONFIG_EXAMPLE_RESET_PROVISIONED=y` to wipe NVS on every boot.

## Test features

Each togglable test behaviour is a Kconfig option named `CONFIG_TESTFW_<NAME>` under **Test firmware features** in `menuconfig`. Every option's help text links the package issue/PR that motivates it. With an option **off**, the firmware behaves like the stock upstream example, so you can reproduce the unpatched behaviour on demand.

| Option | Default | What it does | Motivated by |
| ------ | ------- | ------------ | ------------ |
| `CONFIG_TESTFW_APPLY_BSSID` | on | Honour the BSSID/channel from the provisioning request so the ESP32 joins the specific AP requested | [`esp_provisioning_ble#15`](https://github.com/ESP-Provisioning-BLE/esp_provisioning_ble/pull/15) |

## Contributing / issues

Because this is a test target, the issue tracker is scoped to testing the package. If your request is about the Dart package itself, or is general ESP-IDF help, the issue chooser points you to the right place. In-scope contributions (new `CONFIG_TESTFW_*` test cases, fixes to the firmware) are welcome; see [`AGENTS.md`](AGENTS.md).

## License

[MIT](LICENSE). The provisioning app derives from the ESP-IDF `wifi_prov_mgr` example, which is public domain / CC0.
