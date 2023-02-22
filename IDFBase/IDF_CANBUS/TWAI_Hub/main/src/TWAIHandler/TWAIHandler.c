#include "TWAIHandler.h"

const twai_timing_config_t timingConfig = TWAI_TIMING_CONFIG_25KBITS();
const twai_filter_config_t filterConfig = TWAI_FILTER_CONFIG_ACCEPT_ALL();
const twai_general_config_t generalConfig = TWAI_GENERAL_CONFIG_DEFAULT(TWAI_TX, TWAI_RX, TWAI_MODE_NORMAL);
const twai_message_t pingMessage = {.identifier = ID_MASTER_PING, .data_length_code = 0, .ss = 1, .data = {0, 0, 0, 0, 0, 0, 0, 0}};
const twai_message_t requestMessage = {.identifier = ID_MASTER_REQUEST, .data_length_code = 0, .data = {0, 0, 0, 0, 0, 0, 0, 0}};
const twai_message_t doneMessage = {.identifier = ID_MASTER_DONE, .data_length_code = 0, .data = {0, 0, 0, 0, 0, 0, 0, 0}};

esp_err_t startTWAI()
{
    return twai_driver_install(&generalConfig, &timingConfig, &filterConfig);
}
esp_err_t endTWAI()
{
    return twai_driver_uninstall();
}