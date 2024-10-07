#include "loctek.h"
#include "esphome/core/log.h"
#include "esphome/core/hal.h"
#include "esphome/core/helpers.h"

namespace esphome
{
  namespace loctek
  {

    static const char *const TAG = "loctek";

    void LoctekComponent::loop()
    {
      while (available() > 0)
      {
        byte incomingByte = read();

        // Handle first byte: start of a packet
        if (incomingByte == 0x9b)
        {
          this->msg_len_ = 0;
          this->valid_ = false;
        }

        // Update history buffer
        this->update_history_(incomingByte);

        // Parse packet based on history state
        if (this->history_[0] == 0x9b)
        {
          this->handle_packet_start_(incomingByte);
        }
        else if (this->history_[1] == 0x9b)
        {
          this->handle_packet_length_(incomingByte);
        }
        else if (this->history_[2] == 0x9b)
        {
          this->handle_message_type_(incomingByte);
        }
        else if (this->history_[3] == 0x9b)
        {
          this->handle_first_height_digit_(incomingByte);
        }
        else if (this->history_[4] == 0x9b && this->valid_)
        {
          this->calculate_height_(incomingByte);
        }

        // Handle end byte
        if (incomingByte == 0x9d && this->height_val_ && this->height_val_ != this->lastPublished_)
        {
          this->height_->publish_state(this->height_val_);
          this->lastPublished_ = this->height_val_;
        }
      }
    }

    void LoctekComponent::dump_config()
    {

      ESP_LOGCONFIG(TAG, "Loctek:");

      if (this->is_failed())
      {
        ESP_LOGE(TAG, "Communication with Loctek failed!");
      }

      LOG_SENSOR("  ", "HEIGHT", this->height_);

      this->check_uart_settings(9600);
    }

    // Helper functions
    void LoctekComponent::update_history_(byte b)
    {
      for (int i = 4; i > 0; --i)
      {
        this->history_[i] = this->history_[i - 1];
      }
      this->history_[0] = b;
    }

    void LoctekComponent::handle_packet_start_(byte b)
    {
      // Set message length if the first byte is 0x9b
      this->msg_len_ = static_cast<int>(b);
    }

    void LoctekComponent::handle_packet_length_(byte b)
    {
      this->msg_type_ = b;
    }

    void LoctekComponent::handle_message_type_(byte b)
    {
      if (this->msg_type_ == 0x12 && (this->msg_len_ == 7 || this->msg_len_ == 10))
      {
        if (b == 0 || this->hex_to_int_(b) == 0)
        {
          // Invalid height, handle accordingly (logging or command wakeup could be added here)
        }
        else
        {
          this->valid_ = true;
        }
      }
    }

    void LoctekComponent::handle_first_height_digit_(byte b)
    {
      if (this->valid_)
      {
        // Height digit 2 can be processed here (optional logging)
      }
    }

    void LoctekComponent::calculate_height_(byte b)
    {
      int height1 = this->hex_to_int_(this->history_[1]) * 100;
      int height2 = this->hex_to_int_(this->history_[0]) * 10;
      int height3 = this->hex_to_int_(b);

      if (height2 != 100)
      {
        float finalHeight = height1 + height2 + height3;
        if (this->is_decimal_(this->history_[0]))
        {
          finalHeight /= 10;
        }
        this->height_val_ = finalHeight;
      }
    }

    bool LoctekComponent::is_decimal_(byte b)
    {
      return (b & 0x80) == 0x80;
    }

    int LoctekComponent::hex_to_int_(byte s)
    {

      /*
      This method converts the hexadecimal value provided from the Loctek control box
      to actual integers.

      The loctek control box sends the direct signals that would normally drive a 7-segment display,
      so the bits here correspond to the following segments:

      -- 00 --
      05 -- 01
      05 -- 01
      -- 06 --
      04 -- 02
      04 -- 02
      -- 03 -- 07
      */

      switch (s)
      {
      case 0x7E:
        return 0;
      case 0x06:
        return 1;
      case 0x6D:
        return 2;
      case 0x79:
        return 3;
      case 0x33:
        return 4;
      case 0x5B:
        return 5;
      case 0x5F:
        return 6;
      case 0x70:
        return 7;
      case 0x7F:
        return 8;
      case 0x7B:
        return 9;
      case 0x01:
        return 10;
      default:
        return 0;
      }
    }

  } // namespace loctek
} // namespace esphome