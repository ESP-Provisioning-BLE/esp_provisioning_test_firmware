set(srcs "app_main.c")
set(testfw_opts "")

# TESTFW_APPLY_BSSID (esp_provisioning_ble#15) is implemented by wrapping the
# wifi_provisioning component's get_wifi_prov_handlers() at link time. The
# source, the --wrap flag and WHOLE_ARCHIVE are added ONLY when the option is
# on; with it off the firmware links exactly like the stock example.
#
# WHOLE_ARCHIVE is required because the only reference to
# __wrap_get_wifi_prov_handlers comes from manager.c (in libwifi_provisioning.a,
# a different archive) via --wrap. Without it the linker never pulls
# testfw_apply_bssid.c.obj out of libmain.a and the symbol is undefined. See
# main/testfw_apply_bssid.c and AGENTS.md.
if(CONFIG_TESTFW_APPLY_BSSID)
    list(APPEND srcs "testfw_apply_bssid.c")
    set(testfw_opts WHOLE_ARCHIVE)
endif()

idf_component_register(SRCS ${srcs}
                       PRIV_REQUIRES esp_wifi nvs_flash wifi_provisioning
                       INCLUDE_DIRS "."
                       ${testfw_opts})

if(CONFIG_TESTFW_APPLY_BSSID)
    target_link_libraries(${COMPONENT_LIB} INTERFACE "-Wl,--wrap=get_wifi_prov_handlers")
endif()
