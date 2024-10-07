#pragma once

#include <bitset>
#include "esphome/components/sensor/sensor.h"
#include "esphome/components/uart/uart.h"
#include "esphome/core/component.h"

namespace esphome
{
  namespace loctek
  {

    class LoctekComponent : public Component, public uart::UARTDevice
    {
    public:
      LoctekComponent() = default;

      void loop() override;
      void dump_config() override;
      void setup() override {};

      void set_height(sensor::Sensor *height) { height_ = height; }
      float get_setup_priority() const override { return setup_priority::DATA; }

    protected:
      float height_val_ = 0;
      float lastPublished_ = -1;
      unsigned long history_[5];

      int msg_len_ = 0;
      unsigned long msg_type_;
      bool valid_ = false;

      sensor::Sensor *height_{nullptr};

      int hex_to_int_(byte b);
      bool is_decimal_(byte b);
      void update_history_(byte b);
      void handle_packet_start_(byte b);
      void handle_packet_length_(byte b);
      void handle_message_type_(byte b);
      void handle_first_height_digit_(byte b);
      void calculate_height_(byte b);
    };

  } // namespace loctek
} // namespace esphome