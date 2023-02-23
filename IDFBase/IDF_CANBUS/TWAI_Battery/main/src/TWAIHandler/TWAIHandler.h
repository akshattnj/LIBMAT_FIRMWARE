#ifndef TWAI_HANDLER_H
#define TWAI_HANDLER_H

#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <esp_err.h>
#include <esp_log.h>
#include <driver/twai.h>
#include <string.h>

#include "../definations.h"

esp_err_t startTWAI();
esp_err_t endTWAI();

void taskReceiveTWAI(void *params);
void taskSendTWAI(void *params);

#endif