# esp_provisioning_test_firmware

Guide for agents and contributors. This is the single source of truth for how to work in this repo (`CLAUDE.md` just imports it).

## What this is (and what it is not)

This is **test firmware**: a small, reproducible ESP32 target used to verify the Flutter/Dart package [`esp_provisioning_ble`](https://github.com/ESP-Provisioning-BLE/esp_provisioning_ble) against real hardware over BLE provisioning.

- It **starts from** the official ESP-IDF example `examples/provisioning/wifi_prov_mgr` and is customised from there. It is **not a fork** of `espressif/esp-idf` and is not kept in sync with it.
- It is **not production firmware** and has not been reviewed as such. The only goal is to make package behaviours easy to reproduce and observe.
- **Scope:** only changes that help test `esp_provisioning_ble` belong here. For out-of-scope requests, thank the reporter, explain the scope, and point them at the package repo, the upstream ESP-IDF example, or the Espressif forum (the issue chooser already lists these).

## Hardware and toolchain

- **Target: ESP32-C6** (DevKitM-1 is what we test on). Select it with `idf.py set-target esp32c6`. Other BLE-capable ESP32s should work, but C6 is the one we exercise. (The ESP32-S2 will not work: no BLE.)
- **ESP-IDF is pinned to v5.5.4.** This is deliberate and is **not** to be bumped casually:
  - Since mid-2020 Espressif gives every release the same 30-month support window; as of this repo's creation v5.5 is both the newest 5.x line and the one with the longest remaining support (EOL early 2028), so "newest" and "best supported" coincide.
  - v6.x is intentionally avoided: the `wifi_provisioning` component was split into `network_provisioning` in 6.x, an architectural change that needs its own evaluation rather than being folded in here.
  - This is a **single pinned version, not a matrix.** If a second version is ever wanted to catch behavioural differences, the natural candidate is v5.4.4, but that is a separate decision.

### Local environment on the maintainer's machine

- v5.5.4 lives at `~/.espressif/v5.5.4/esp-idf`. Activate it with `. ~/.espressif/v5.5.4/esp-idf/export.sh`.
- A v5.5.2 install also exists (`~/.espressif/v5.5.2/esp-idf`) and can be used as a fallback: the `wifi_provisioning` behaviour that motivates this repo is byte-identical between v5.5.2 and v5.5.4.

## Build, flash, monitor

```bash
. ~/.espressif/v5.5.4/esp-idf/export.sh   # activate the toolchain
idf.py set-target esp32c6                 # once per checkout / target change
idf.py build                              # uses sdkconfig.defaults automatically
idf.py -p <PORT> flash monitor            # flash + open serial monitor
```

- The BLE device name is `PROV_XXXXXX` (last 3 bytes of the MAC), printed in the log as `Provisioning started ... service name : PROV_XXXXXX`. It is generated in `app_main.c` (`get_device_service_name`), not configurable via menuconfig.
- **Re-provisioning:** once provisioned, the device stops advertising on the next boot (credentials are in NVS). To test again from scratch: `idf.py -p <PORT> erase-flash` then re-flash, or set `CONFIG_EXAMPLE_RESET_PROVISIONED=y` so NVS is wiped on every boot.
- `Ctrl-]` exits the monitor without resetting the board.

## The two gotchas this firmware bakes in

These are the exact things that are painful to rediscover by hand each time, so they are encoded here once:

1. **The example defaults to Security 2, which the package does not support yet (`esp_provisioning_ble#30`).** With Security 2 the handshake never completes and it looks like a package bug. `sdkconfig.defaults` forces `CONFIG_EXAMPLE_PROV_SECURITY_VERSION_1`. With Security 1 the proof-of-possession is hard-coded to `"abcd1234"` in `app_main.c`, which matches the value the package's own example pre-fills, so nothing has to be typed.

   The security choice also offers **Security 0** (plain text, no PoP), which the upstream example does not: it is this repo's only change to the inherited "Example Configuration" menu, added to test Security0 in the package (`esp_provisioning_ble#31`). Build it by chaining `sdkconfig.ci.sec0` after the defaults. Protocomm Security 0 posts no `PROTOCOMM_SECURITY_SESSION_EVENT`, so the `Secured session established!` log never appears with it; that is expected, not a failure.

2. **Upstream silently ignores the BSSID sent by the client.** The `wifi_provisioning` component parses and length-validates the BSSID in `components/wifi_provisioning/src/wifi_config.c` (into a `wifi_prov_config_set_data_t`), but its `set_config_handler` (`src/handlers.c`) only copies the SSID, password and scan method into `wifi_config_t.sta`; the BSSID and channel are dropped. By the time the official `WIFI_PROV_SET_STA_CONFIG` app event fires (`manager.c: wifi_prov_mgr_configure_sta`), the `wifi_config_t` no longer carries the client BSSID, and the config handlers are wired internally (`get_wifi_prov_handlers`), so the app event cannot recover it. This firmware restores the behaviour behind the `CONFIG_TESTFW_APPLY_BSSID` toggle (see below). Motivated by `esp_provisioning_ble#15`.

   **How the fix is wired** (`main/testfw_apply_bssid.c`, only compiled when the option is on): the linker flag `-Wl,--wrap=get_wifi_prov_handlers` lets us keep every upstream handler except `set_config_handler`, which we replace with a faithful copy that additionally copies the BSSID into `wifi_config_t.sta.bssid`, sets `bssid_set = true` and applies the channel. Nothing in `app_main.c` changes, so divergence from the example stays tiny. The opaque `wifi_prov_ctx_t` is completed to match the private definition in `handlers.c`; this is why the ESP-IDF version is pinned (a struct-layout change upstream would need this file revisited).

## Test-feature convention: `CONFIG_TESTFW_<NAME>`

Every togglable test behaviour is one Kconfig option under the **"Test firmware features"** menu in `main/Kconfig.projbuild`. Rules:

- Prefix every option `CONFIG_TESTFW_` (verified not to collide with any Espressif symbol).
- The option's `help` **must link the `esp_provisioning_ble` issue/PR** that motivates it, so the reason is never lost.
- Gate all code with `#if CONFIG_TESTFW_<NAME>`. With the option **off**, the firmware must behave like the stock upstream example (so the toggle can reproduce the unpatched behaviour).
- CI builds the firmware with each feature both on and off.

Current features:

| Option | Default | Motivated by |
| ------ | ------- | ------------ |
| `CONFIG_TESTFW_APPLY_BSSID` | `y` | [`esp_provisioning_ble#15`](https://github.com/ESP-Provisioning-BLE/esp_provisioning_ble/pull/15) |

## Repository layout

```
CMakeLists.txt            top-level project CMake (project name, minimal build)
sdkconfig.defaults        forces BLE transport + Security 1 + NimBLE + partitions
sdkconfig.ci.no_bssid     CI-only override: TESTFW_APPLY_BSSID=n
sdkconfig.ci.sec0         CI-only override: protocomm Security 0 instead of 1
partitions.csv            custom partition table (BLE image is large)
main/
  app_main.c              provisioning app (from the upstream example)
  Kconfig.projbuild       upstream "Example Configuration" menu (+ Security 0) + TESTFW menu
  CMakeLists.txt          component registration
  idf_component.yml       managed deps (qrcode)
.github/workflows/build.yml   build-only CI, pinned ESP-IDF v5.5.4, esp32c6
.github/ISSUE_TEMPLATE/       templates that keep issues in scope
```

## Conventions

- Keep divergence from the upstream example **minimal and localised.** Prefer Kconfig-gated additions over rewriting example code, so it stays easy to see what this repo changed and why.
- When adding a test feature, follow the `CONFIG_TESTFW_<NAME>` rules above.
- Do not add agents/skills/config under `.claude/` unless they are genuinely useful for this repo; `.claude/` is versioned and public.
