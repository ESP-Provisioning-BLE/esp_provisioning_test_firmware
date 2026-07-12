---
name: Test case / reproduction for the Dart package
about: Propose or report a behaviour to verify against real hardware with this firmware
title: "[test] "
labels: ["test-case"]
---

<!--
Thanks for helping test esp_provisioning_ble against real hardware!

Scope reminder: this repository is a *test target* for the Dart package `esp_provisioning_ble`, not general-purpose or production firmware. Good fits here are things that exercise the package over BLE provisioning against an ESP32. If your request is about the package's own code, please use the package repo instead (link in the issue chooser).
-->

## What package behaviour does this exercise?
<!-- Which esp_provisioning_ble feature / issue / PR is this about? Link it. -->

## Proposed test-firmware behaviour
<!-- e.g. "a CONFIG_TESTFW_* toggle that makes the device do X so the package can observe Y". If it maps to a specific ESP-IDF gotcha, describe it. -->

## How would we observe the result?
<!-- Serial log line, connection to a specific AP, an error the package should surface, etc. -->

## Environment
- ESP32 target (e.g. esp32c6):
- ESP-IDF version:
- esp_provisioning_ble version / branch:
