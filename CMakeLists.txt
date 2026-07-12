# Top-level project CMake for esp_provisioning_test_firmware.
#
# This is a standalone ESP-IDF project. It is NOT a fork of esp-idf: it merely
# starts from the shape of the official example
# examples/provisioning/wifi_prov_mgr (public domain / CC0) and is customised
# from there. See AGENTS.md for the rationale.
#
# The following lines of boilerplate have to be in this exact order for CMake to
# work correctly.
cmake_minimum_required(VERSION 3.16)

include($ENV{IDF_PATH}/tools/cmake/project.cmake)

# "Trim" the build: include only the minimal set of components that `main` and
# its dependencies actually need. Inherited from the upstream example.
idf_build_set_property(MINIMAL_BUILD ON)

project(esp_provisioning_test_firmware)
