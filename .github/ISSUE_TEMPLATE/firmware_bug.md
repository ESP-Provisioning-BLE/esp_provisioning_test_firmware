---
name: Bug in the test firmware
about: Something in this firmware (build, flash, a TESTFW toggle) is broken
title: "[bug] "
labels: ["bug"]
---

<!--
Scope reminder: this repository is a *test target* for the Dart package `esp_provisioning_ble`. Please use this template only for problems with the firmware itself (it won't build, a CONFIG_TESTFW_* toggle misbehaves, the device doesn't advertise, etc.). Problems with the Dart package belong in the package repo (link in the issue chooser).
-->

## What went wrong?

## Steps to reproduce
1.
2.

## Expected vs actual

## Environment
- ESP32 target (e.g. esp32c6):
- ESP-IDF version (this repo pins v5.5.4):
- Host OS:
- Relevant CONFIG_TESTFW_* toggles and their values:

## Logs
<!-- Paste the relevant `idf.py monitor` / build output. -->
