#include "nus_service.h"

#include <cstring>
#include <esp_log.h>
#include <esp_bt.h>
#include <esp_bt_main.h>
#include <esp_gap_ble_api.h>
#include <esp_gatts_api.h>
#include <esp_gatt_common_api.h>
#include <esp_bt_device.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>

#define TAG "nus"

// Ring buffer for incoming BLE data
static const size_t RX_CAP = 2048;
static uint8_t  s_rx_buf[RX_CAP];
static volatile size_t s_rx_head = 0;
static volatile size_t s_rx_tail = 0;

static uint16_t s_gatts_if = ESP_GATT_IF_NONE;
static uint16_t s_conn_id = 0;
static uint16_t s_mtu = 23;
static bool s_connected = false;
static bool s_secure = false;
static uint32_t s_passkey = 0;
static const char* s_device_name = nullptr;

// GATT attribute handles
static uint16_t s_service_handle = 0;
static uint16_t s_tx_char_handle = 0;
static uint16_t s_tx_descr_handle = 0;  // CCCD
static uint16_t s_rx_char_handle = 0;
static bool s_tx_notify_enabled = false;

// NUS UUIDs (128-bit, little-endian)
static const uint8_t NUS_SERVICE_UUID[16] = {
    0x9e, 0xca, 0xdc, 0x24, 0x0e, 0xe5, 0xa9, 0xe0,
    0x93, 0xf3, 0xa3, 0xb5, 0x01, 0x00, 0x40, 0x6e
};
static const uint8_t NUS_TX_UUID[16] = {
    0x9e, 0xca, 0xdc, 0x24, 0x0e, 0xe5, 0xa9, 0xe0,
    0x93, 0xf3, 0xa3, 0xb5, 0x03, 0x00, 0x40, 0x6e
};
static const uint8_t NUS_RX_UUID[16] = {
    0x9e, 0xca, 0xdc, 0x24, 0x0e, 0xe5, 0xa9, 0xe0,
    0x93, 0xf3, 0xa3, 0xb5, 0x02, 0x00, 0x40, 0x6e
};

// Attribute creation state machine
enum NusInitStep {
    STEP_CREATE_SERVICE = 0,
    STEP_ADD_TX_CHAR,
    STEP_ADD_TX_CCCD,
    STEP_ADD_RX_CHAR,
    STEP_START_SERVICE,
    STEP_DONE,
};
static NusInitStep s_init_step = STEP_CREATE_SERVICE;

static void rx_push(const uint8_t* p, size_t n) {
    for (size_t i = 0; i < n; i++) {
        size_t next = (s_rx_head + 1) % RX_CAP;
        if (next == s_rx_tail) return;
        s_rx_buf[s_rx_head] = p[i];
        s_rx_head = next;
    }
}

static void start_advertising() {
    esp_ble_adv_data_t adv_data = {};
    adv_data.set_scan_rsp = false;
    adv_data.include_name = true;
    adv_data.include_txpower = false;
    adv_data.min_interval = 0x0006;
    adv_data.max_interval = 0x0012;
    adv_data.appearance = 0;
    adv_data.manufacturer_len = 0;
    adv_data.service_data_len = 0;
    adv_data.service_uuid_len = 16;
    adv_data.p_service_uuid = (uint8_t*)NUS_SERVICE_UUID;
    adv_data.flag = (ESP_BLE_ADV_FLAG_GEN_DISC | ESP_BLE_ADV_FLAG_BREDR_NOT_SPT);

    esp_ble_gap_config_adv_data(&adv_data);

    // Also set scan response with the service UUID
    esp_ble_adv_data_t scan_rsp = {};
    scan_rsp.set_scan_rsp = true;
    scan_rsp.include_name = false;
    scan_rsp.service_uuid_len = 16;
    scan_rsp.p_service_uuid = (uint8_t*)NUS_SERVICE_UUID;
    esp_ble_gap_config_adv_data(&scan_rsp);
}

static void gap_event_handler(esp_gap_ble_cb_event_t event, esp_ble_gap_cb_param_t* param) {
    switch (event) {
        case ESP_GAP_BLE_ADV_DATA_SET_COMPLETE_EVT:
        case ESP_GAP_BLE_ADV_DATA_RAW_SET_COMPLETE_EVT: {
            esp_ble_adv_params_t adv_params = {};
            adv_params.adv_int_min = 0x20;
            adv_params.adv_int_max = 0x40;
            adv_params.adv_type = ADV_TYPE_IND;
            adv_params.own_addr_type = BLE_ADDR_TYPE_PUBLIC;
            adv_params.channel_map = ADV_CHNL_ALL;
            adv_params.adv_filter_policy = ADV_FILTER_ALLOW_SCAN_ANY_CON_ANY;
            esp_ble_gap_start_advertising(&adv_params);
            break;
        }
        case ESP_GAP_BLE_PASSKEY_NOTIF_EVT:
            s_passkey = param->ble_security.key_notif.passkey;
            ESP_LOGI(TAG, "Passkey: %06lu", (unsigned long)s_passkey);
            break;
        case ESP_GAP_BLE_AUTH_CMPL_EVT:
            s_passkey = 0;
            s_secure = param->ble_security.auth_cmpl.success;
            ESP_LOGI(TAG, "Auth complete: %s", s_secure ? "ok" : "FAIL");
            if (!s_secure && s_connected) {
                esp_ble_gatts_close(s_gatts_if, s_conn_id);
            }
            break;
        case ESP_GAP_BLE_SEC_REQ_EVT:
            esp_ble_gap_security_rsp(param->ble_security.ble_req.bd_addr, true);
            break;
        default:
            break;
    }
}

static void gatts_event_handler(esp_gatts_cb_event_t event, esp_gatt_if_t gatts_if,
                                esp_ble_gatts_cb_param_t* param) {
    switch (event) {
        case ESP_GATTS_REG_EVT:
            if (param->reg.status == ESP_GATT_OK) {
                s_gatts_if = gatts_if;
                esp_ble_gap_set_device_name(s_device_name);
                esp_ble_gatts_set_attr_value(0, 0, nullptr);  // dummy

                // Step 1: create service
                esp_gatt_srvc_id_t service_id = {};
                service_id.is_primary = true;
                service_id.id.inst_id = 0;
                service_id.id.uuid.len = ESP_UUID_LEN_128;
                memcpy(service_id.id.uuid.uuid.uuid128, NUS_SERVICE_UUID, 16);
                // 4 handles: service + TX char + TX CCCD + RX char (each char = char decl + char val)
                esp_ble_gatts_create_service(gatts_if, &service_id, 8);
            }
            break;

        case ESP_GATTS_CREATE_EVT:
            if (param->create.status == ESP_GATT_OK) {
                s_service_handle = param->create.service_handle;
                s_init_step = STEP_ADD_TX_CHAR;

                // Add TX characteristic (Notify, encrypted read)
                esp_bt_uuid_t tx_uuid = {};
                tx_uuid.len = ESP_UUID_LEN_128;
                memcpy(tx_uuid.uuid.uuid128, NUS_TX_UUID, 16);
                esp_gatt_char_prop_t tx_prop = ESP_GATT_CHAR_PROP_BIT_NOTIFY;
                esp_ble_gatts_add_char(s_service_handle, &tx_uuid,
                    ESP_GATT_PERM_READ_ENCRYPTED,
                    tx_prop, nullptr, nullptr);
            }
            break;

        case ESP_GATTS_ADD_CHAR_EVT:
            if (param->add_char.status == ESP_GATT_OK) {
                if (s_init_step == STEP_ADD_TX_CHAR) {
                    s_tx_char_handle = param->add_char.attr_handle;
                    s_init_step = STEP_ADD_TX_CCCD;

                    // Add CCCD descriptor for TX characteristic
                    esp_bt_uuid_t cccd_uuid = {};
                    cccd_uuid.len = ESP_UUID_LEN_16;
                    cccd_uuid.uuid.uuid16 = ESP_GATT_UUID_CHAR_CLIENT_CONFIG;
                    esp_ble_gatts_add_char_descr(s_service_handle, &cccd_uuid,
                        ESP_GATT_PERM_READ_ENCRYPTED | ESP_GATT_PERM_WRITE_ENCRYPTED,
                        nullptr, nullptr);

                } else if (s_init_step == STEP_ADD_RX_CHAR) {
                    s_rx_char_handle = param->add_char.attr_handle;
                    s_init_step = STEP_START_SERVICE;

                    // Start service
                    esp_ble_gatts_start_service(s_service_handle);
                }
            }
            break;

        case ESP_GATTS_ADD_CHAR_DESCR_EVT:
            if (param->add_char_descr.status == ESP_GATT_OK && s_init_step == STEP_ADD_TX_CCCD) {
                s_tx_descr_handle = param->add_char_descr.attr_handle;
                s_init_step = STEP_ADD_RX_CHAR;

                // Add RX characteristic (Write + Write No Response, encrypted)
                esp_bt_uuid_t rx_uuid = {};
                rx_uuid.len = ESP_UUID_LEN_128;
                memcpy(rx_uuid.uuid.uuid128, NUS_RX_UUID, 16);
                esp_gatt_char_prop_t rx_prop = ESP_GATT_CHAR_PROP_BIT_WRITE | ESP_GATT_CHAR_PROP_BIT_WRITE_NR;
                esp_ble_gatts_add_char(s_service_handle, &rx_uuid,
                    ESP_GATT_PERM_WRITE_ENCRYPTED,
                    rx_prop, nullptr, nullptr);
            }
            break;

        case ESP_GATTS_START_EVT:
            if (param->start.status == ESP_GATT_OK) {
                s_init_step = STEP_DONE;
                ESP_LOGI(TAG, "NUS service started, advertising...");
                start_advertising();
            }
            break;

        case ESP_GATTS_CONNECT_EVT:
            s_conn_id = param->connect.conn_id;
            s_connected = true;
            s_secure = false;
            s_tx_notify_enabled = false;
            ESP_LOGI(TAG, "Connected, conn_id=%d", s_conn_id);
            // Request security
            esp_ble_set_encryption(param->connect.remote_bda, ESP_BLE_SEC_ENCRYPT_MITM);
            break;

        case ESP_GATTS_DISCONNECT_EVT:
            s_connected = false;
            s_secure = false;
            s_passkey = 0;
            s_mtu = 23;
            s_tx_notify_enabled = false;
            ESP_LOGI(TAG, "Disconnected");
            start_advertising();
            break;

        case ESP_GATTS_MTU_EVT:
            s_mtu = param->mtu.mtu;
            ESP_LOGI(TAG, "MTU=%u", s_mtu);
            break;

        case ESP_GATTS_WRITE_EVT:
            ESP_LOGI(TAG, "Write: handle=%d len=%d need_rsp=%d is_prep=%d",
                     param->write.handle, param->write.len,
                     param->write.need_rsp, param->write.is_prep);
            if (param->write.handle == s_rx_char_handle) {
                rx_push(param->write.value, param->write.len);
                ESP_LOGI(TAG, "RX %d bytes, ring avail=%d", param->write.len,
                         (int)((s_rx_head + RX_CAP - s_rx_tail) % RX_CAP));
                if (param->write.need_rsp) {
                    esp_ble_gatts_send_response(gatts_if, param->write.conn_id,
                        param->write.trans_id, ESP_GATT_OK, nullptr);
                }
            } else if (param->write.handle == s_tx_descr_handle) {
                if (param->write.len == 2) {
                    uint16_t cccd_val = param->write.value[0] | (param->write.value[1] << 8);
                    s_tx_notify_enabled = (cccd_val & 0x0001) != 0;
                    ESP_LOGI(TAG, "TX notifications %s", s_tx_notify_enabled ? "enabled" : "disabled");
                }
                if (param->write.need_rsp) {
                    esp_ble_gatts_send_response(gatts_if, param->write.conn_id,
                        param->write.trans_id, ESP_GATT_OK, nullptr);
                }
            } else {
                ESP_LOGW(TAG, "Write to unknown handle %d (rx=%d tx_cccd=%d)",
                         param->write.handle, s_rx_char_handle, s_tx_descr_handle);
                if (param->write.need_rsp) {
                    esp_ble_gatts_send_response(gatts_if, param->write.conn_id,
                        param->write.trans_id, ESP_GATT_OK, nullptr);
                }
            }
            break;

        case ESP_GATTS_READ_EVT:
            // Handle read requests with an empty response
            if (param->read.need_rsp) {
                esp_gatt_rsp_t rsp = {};
                rsp.attr_value.handle = param->read.handle;
                rsp.attr_value.len = 0;
                esp_ble_gatts_send_response(gatts_if, param->read.conn_id,
                    param->read.trans_id, ESP_GATT_OK, &rsp);
            }
            break;

        default:
            break;
    }
}

void nus_init(const char* device_name) {
    s_device_name = device_name;
    s_rx_head = 0;
    s_rx_tail = 0;

    // Release classic BT memory (BLE only) — not available on all chips
#if CONFIG_IDF_TARGET_ESP32 || CONFIG_IDF_TARGET_ESP32S3
    esp_bt_controller_mem_release(ESP_BT_MODE_CLASSIC_BT);
#endif

    esp_bt_controller_config_t bt_cfg = BT_CONTROLLER_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_bt_controller_init(&bt_cfg));
    ESP_ERROR_CHECK(esp_bt_controller_enable(ESP_BT_MODE_BLE));
    ESP_ERROR_CHECK(esp_bluedroid_init());
    ESP_ERROR_CHECK(esp_bluedroid_enable());

    ESP_ERROR_CHECK(esp_ble_gatts_register_callback(gatts_event_handler));
    ESP_ERROR_CHECK(esp_ble_gap_register_callback(gap_event_handler));

    // Security parameters
    esp_ble_auth_req_t auth_req = ESP_LE_AUTH_REQ_SC_MITM_BOND;
    esp_ble_io_cap_t io_cap = ESP_IO_CAP_OUT;
    uint8_t key_size = 16;
    uint8_t init_key = ESP_BLE_ENC_KEY_MASK | ESP_BLE_ID_KEY_MASK;
    uint8_t rsp_key = ESP_BLE_ENC_KEY_MASK | ESP_BLE_ID_KEY_MASK;
    esp_ble_gap_set_security_param(ESP_BLE_SM_AUTHEN_REQ_MODE, &auth_req, sizeof(auth_req));
    esp_ble_gap_set_security_param(ESP_BLE_SM_IOCAP_MODE, &io_cap, sizeof(io_cap));
    esp_ble_gap_set_security_param(ESP_BLE_SM_MAX_KEY_SIZE, &key_size, sizeof(key_size));
    esp_ble_gap_set_security_param(ESP_BLE_SM_SET_INIT_KEY, &init_key, sizeof(init_key));
    esp_ble_gap_set_security_param(ESP_BLE_SM_SET_RSP_KEY, &rsp_key, sizeof(rsp_key));

    // Request max MTU (macOS typically negotiates ~185)
    esp_ble_gatt_set_local_mtu(517);

    // Register GATTS app — triggers ESP_GATTS_REG_EVT which starts service creation
    ESP_ERROR_CHECK(esp_ble_gatts_app_register(0));

    ESP_LOGI(TAG, "NUS BLE initialized as '%s'", device_name);
}

bool nus_connected() { return s_connected; }
bool nus_secure() { return s_secure; }
uint32_t nus_passkey() { return s_passkey; }

void nus_clear_bonds() {
    int dev_num = esp_ble_get_bond_device_num();
    if (dev_num <= 0) return;
    esp_ble_bond_dev_t* devs = (esp_ble_bond_dev_t*)malloc(sizeof(esp_ble_bond_dev_t) * dev_num);
    if (!devs) return;
    esp_ble_get_bond_device_list(&dev_num, devs);
    for (int i = 0; i < dev_num; i++) {
        esp_ble_remove_bond_device(devs[i].bd_addr);
    }
    free(devs);
    ESP_LOGI(TAG, "Cleared %d bond(s)", dev_num);
}

size_t nus_available() {
    return (s_rx_head + RX_CAP - s_rx_tail) % RX_CAP;
}

int nus_read() {
    if (s_rx_head == s_rx_tail) return -1;
    int b = s_rx_buf[s_rx_tail];
    s_rx_tail = (s_rx_tail + 1) % RX_CAP;
    return b;
}

size_t nus_write(const uint8_t* data, size_t len) {
    if (!s_connected || !s_tx_notify_enabled || s_tx_char_handle == 0) return 0;

    size_t chunk = (s_mtu > 3) ? s_mtu - 3 : 20;
    if (chunk > 180) chunk = 180;
    size_t sent = 0;

    while (sent < len) {
        size_t n = (len - sent > chunk) ? chunk : (len - sent);
        esp_ble_gatts_send_indicate(s_gatts_if, s_conn_id, s_tx_char_handle,
                                    n, (uint8_t*)(data + sent), false);
        sent += n;
        if (sent < len) vTaskDelay(pdMS_TO_TICKS(4));
    }
    return sent;
}
