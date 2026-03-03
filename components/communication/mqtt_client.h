/**
 * MQTT Client - Header File
 */

#ifndef POULTRY_MQTT_CLIENT_H
#define POULTRY_MQTT_CLIENT_H

#include <esp_err.h>
#include <mqtt_client.h>  /* ESP-IDF MQTT types (esp_mqtt_client_handle_t, etc.) */

/**
 * @brief Initialize MQTT client (without authentication)
 */
esp_err_t mqtt_client_init(const char *broker_uri);

/**
 * @brief Initialize MQTT client with authentication
 */
esp_err_t mqtt_client_init_auth(const char *broker_uri, const char *username, const char *password);

/**
 * @brief Publish message
 */
esp_err_t mqtt_client_publish(const char *topic, const char *data, int len);

/**
 * @brief Subscribe to topic
 */
esp_err_t mqtt_client_subscribe(const char *topic);

#endif /* POULTRY_MQTT_CLIENT_H */
