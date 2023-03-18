#ifndef CAN_SENDER_H_
#define CAN_SENDER_H_

#include "driver/can.h"

class CanSender {
  public:
    CanSender(gpio_num_t tx_pin, gpio_num_t rx_pin, can_general_config_t g_config, can_timing_config_t t_config, can_filter_config_t f_config);
    void StartSending(uint32_t identifier1, uint32_t identifier2, uint8_t data1, uint8_t data2, TickType_t delay);
  private:
    can_general_config_t g_config_;
    can_timing_config_t t_config_;
    can_filter_config_t f_config_;
};

#endif  // CAN_SENDER_H_
