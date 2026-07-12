/*
 * TESTFW_APPLY_BSSID — honour the BSSID/channel sent in the provisioning
 * SetConfig request (esp_provisioning_ble#15).
 *
 * SPDX-License-Identifier: MIT
 *
 * -----------------------------------------------------------------------------
 * Why this file exists
 * -----------------------------------------------------------------------------
 * ESP-IDF's wifi_provisioning component parses and length-validates the BSSID
 * and channel that the client sends in the SetConfig request (into a
 * `wifi_prov_config_set_data_t`, see components/wifi_provisioning/src/
 * wifi_config.c), but its `set_config_handler`
 * (components/wifi_provisioning/src/handlers.c) copies only the SSID, password
 * and scan method into `wifi_config_t.sta` and drops the BSSID and channel. By
 * the time the official `WIFI_PROV_SET_STA_CONFIG` app event fires
 * (manager.c: wifi_prov_mgr_configure_sta), the STA config no longer carries
 * the client BSSID, so the event handler cannot recover it. The config handlers
 * are wired internally via the non-public `get_wifi_prov_handlers()` and the
 * public API exposes only the event callback.
 *
 * -----------------------------------------------------------------------------
 * How it works
 * -----------------------------------------------------------------------------
 * This file is compiled, and the linker flag `-Wl,--wrap=get_wifi_prov_handlers`
 * is added, ONLY when CONFIG_TESTFW_APPLY_BSSID is set (see main/CMakeLists.txt).
 * The wrap lets us keep every upstream handler except `set_config_handler`,
 * which we replace with a faithful copy of the upstream one that additionally
 * applies the BSSID and channel. With the option off, none of this is built and
 * the firmware behaves exactly like the stock example.
 *
 * The opaque `wifi_prov_ctx_t` is completed here to match the private definition
 * in handlers.c (a single `wifi_config_t`). This is safe because the upstream
 * `apply_config_handler` / `get_status_handler` we keep read the very same
 * layout. It is deliberately pinned to ESP-IDF v5.5.4 (see AGENTS.md).
 */

#include <string.h>
#include <stdlib.h>

#include <esp_err.h>
#include <esp_log.h>
#include <esp_wifi.h>

#include <wifi_provisioning/wifi_config.h>

static const char *TAG = "testfw_bssid";

/* Complete the opaque context type exactly as handlers.c defines it. */
struct wifi_prov_ctx {
    wifi_config_t wifi_cfg;
};

/* --- helpers copied verbatim from upstream handlers.c ------------------------ */

static wifi_config_t *get_config(wifi_prov_ctx_t **ctx)
{
    return (*ctx ? &(*ctx)->wifi_cfg : NULL);
}

static wifi_config_t *new_config(wifi_prov_ctx_t **ctx)
{
    free(*ctx);
    (*ctx) = (wifi_prov_ctx_t *) calloc(1, sizeof(wifi_prov_ctx_t));
    return get_config(ctx);
}

static void free_config(wifi_prov_ctx_t **ctx)
{
    free(*ctx);
    *ctx = NULL;
}

/* --- our set_config_handler: upstream behaviour + apply BSSID/channel -------- */

static esp_err_t testfw_set_config_handler(const wifi_prov_config_set_data_t *req_data,
                                           wifi_prov_ctx_t **ctx)
{
    wifi_config_t *wifi_cfg = get_config(ctx);
    if (wifi_cfg) {
        free_config(ctx);
    }

    wifi_cfg = new_config(ctx);
    if (!wifi_cfg) {
        ESP_LOGE(TAG, "Unable to allocate Wi-Fi config");
        return ESP_ERR_NO_MEM;
    }

    ESP_LOGD(TAG, "Wi-Fi Credentials Received");

    /* SSID: memcpy over a zeroed buffer so a <32-byte SSID is NUL terminated. */
    const size_t ssid_len = strnlen(req_data->ssid, sizeof(wifi_cfg->sta.ssid));
    memset(wifi_cfg->sta.ssid, 0, sizeof(wifi_cfg->sta.ssid));
    memcpy(wifi_cfg->sta.ssid, req_data->ssid, ssid_len);

    /* Password: strlcpy guarantees NUL termination (buffer has the extra byte). */
    strlcpy((char *) wifi_cfg->sta.password, req_data->password, sizeof(wifi_cfg->sta.password));

#ifdef CONFIG_WIFI_PROV_STA_ALL_CHANNEL_SCAN
    wifi_cfg->sta.scan_method = WIFI_ALL_CHANNEL_SCAN;
#else /* CONFIG_WIFI_PROV_STA_FAST_SCAN */
    wifi_cfg->sta.scan_method = WIFI_FAST_SCAN;
#endif

    /* --- the reason this file exists: apply the client BSSID/channel ---------
     * A BSSID of all-zeros means the client did not send one, so only honour a
     * real BSSID (otherwise we would pin the STA to 00:00:00:00:00:00). */
    static const uint8_t zero_bssid[6] = {0};
    if (memcmp(req_data->bssid, zero_bssid, sizeof(req_data->bssid)) != 0) {
        memcpy(wifi_cfg->sta.bssid, req_data->bssid, sizeof(wifi_cfg->sta.bssid));
        wifi_cfg->sta.bssid_set = true;
        wifi_cfg->sta.channel = req_data->channel;
        ESP_LOGI(TAG,
                 "TESTFW_APPLY_BSSID: honouring client BSSID %02x:%02x:%02x:%02x:%02x:%02x (channel %u)",
                 wifi_cfg->sta.bssid[0], wifi_cfg->sta.bssid[1], wifi_cfg->sta.bssid[2],
                 wifi_cfg->sta.bssid[3], wifi_cfg->sta.bssid[4], wifi_cfg->sta.bssid[5],
                 req_data->channel);
    }

    return ESP_OK;
}

/* --- the wrap ---------------------------------------------------------------
 * `get_wifi_prov_handlers` is declared in the component's private header
 * (wifi_provisioning_priv.h); declare the __real_ alias ourselves. */
esp_err_t __real_get_wifi_prov_handlers(wifi_prov_config_handlers_t *ptr);

esp_err_t __wrap_get_wifi_prov_handlers(wifi_prov_config_handlers_t *ptr)
{
    esp_err_t err = __real_get_wifi_prov_handlers(ptr);
    if (err != ESP_OK) {
        return err;
    }
    /* Keep upstream get_status/apply/ctx; only override set_config_handler. */
    ptr->set_config_handler = testfw_set_config_handler;
    return ESP_OK;
}
