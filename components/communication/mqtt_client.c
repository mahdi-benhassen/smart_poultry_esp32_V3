/**
 * MQTT Client - Implementation
 */

#include <string.h>
#include <esp_log.h>
#include "mqtt_client.h"   /* Project header (includes ESP-IDF mqtt_client.h) */

static const char *TAG = "MQTT_CLIENT";
static esp_mqtt_client_handle_t s_mqtt_client = NULL;

/**
 * @brief Initialize MQTT client
 */
esp_err_t mqtt_client_init(const char *broker_uri)
{
    return mqtt_client_init_auth(broker_uri, NULL, NULL);
}

/**
 * @brief Initialize MQTT client with authentication
 */
esp_err_t mqtt_client_init_auth(const char *broker_uri, const char *username, const char *password)
{
    esp_mqtt_client_config_t mqtt_cfg = {
        .broker = {
            .address = {
                .uri = broker_uri,
            },
        },
        .credentials = {
            .client_id = "esp32_poultry",
        },
    };

    /* Add authentication if provided */
    if (username != NULL && password != NULL) {
        mqtt_cfg.credentials.username = username;
        mqtt_cfg.credentials.authentication.password = password;
    }

    s_mqtt_client = esp_mqtt_client_init(&mqtt_cfg);
    if (s_mqtt_client == NULL) {
        return ESP_FAIL;
    }

    ESP_ERROR_CHECK(esp_mqtt_client_start(s_mqtt_client));

    ESP_LOGI(TAG, "MQTT client initialized");

    return ESP_OK;
}

/**
 * @brief Publish message
 */
esp_err_t mqtt_client_publish(const char *topic, const char *data, int len)
{
    if (!s_mqtt_client) return ESP_ERR_INVALID_STATE;

    int msg_id = esp_mqtt_client_publish(s_mqtt_client, topic, data, len, 1, 0);
    return (msg_id >= 0) ? ESP_OK : ESP_FAIL;
}

/**
 * @brief Subscribe to topic
 */
esp_err_t mqtt_client_subscribe(const char *topic)
{
    if (!s_mqtt_client) return ESP_ERR_INVALID_STATE;

    int msg_id = esp_mqtt_client_subscribe(s_mqtt_client, topic, 1);
    return (msg_id >= 0) ? ESP_OK : ESP_FAIL;
}
