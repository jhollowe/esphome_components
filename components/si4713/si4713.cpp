#include "si4713.h"

#include "esphome/core/helpers.h"
#include "esphome/core/log.h"

namespace esphome {
namespace si4713 {

// TODO see if this can support multiple devices (up to 2 since there are only 2 I2C addresses)

void Si4713Component::dump_config() {
  ESP_LOGCONFIG(TAG, "Si4713Component");
  LOG_I2C_DEVICE(this);
  LOG_PIN("  Reset Pin: ", this->reset_pin_);
}

void Si4713Component::setup() {
  // // no reset pin, hope it comes up
  // if (this->reset_pin_ == nullptr) {
  //   this->finish_setup_();
  //   return;
  // }

  this->reset_pin_->setup();
  this->reset_();

  this->power_up_();
  rev_info_t info = this->get_info_();
  this->pretty_print_rev_info(info);
  if (info.part_number != 13) {
    ESP_LOGE(TAG, "Device is not Si4713 (part number %u)", info.part_number);
  }
}

void Si4713Component::temp_testing() {
  tune_status_t ts = this->get_tune_status();
  this->print_tune_status(ts);

  asq_status_t asq = this->get_asq_status();
  this->print_asq_status(asq);
}

void Si4713Component::update() { this->temp_testing(); }

void Si4713Component::reset_() {
  // RST needs to be pulled low to reset
  this->reset_pin_->digital_write(true);
  delay(10);
  this->reset_pin_->digital_write(false);
  delay(10);
  this->reset_pin_->digital_write(true);
}

void Si4713Component::power_up_() {
  const uint8_t args[] = {
      // 0 CTS interrupt disabled
      // 0 GPO2 output disabled
      // 0 Boot normally (no firmware patch)
      // 1 crystal oscillator ENabled
      // 0010 function: FM transmit
      0b00010010,
      // analog input mode
      0x50,
      // digital input mode
      // 0x0f
  };
  uint8_t status;
  ESP_LOGV(TAG, "Powering up Si4713...");
  this->write_register(SI4710_CMD_POWER_UP, args, sizeof(args));
  status = this->wait_for_cts_();
  this->print_status(status);
}

// untested
void Si4713Component::power_down_() {
  uint8_t status;
  ESP_LOGV(TAG, "Powering down Si4713...");
  this->write_register(SI4710_CMD_POWER_DOWN, nullptr, 0);
  status = this->wait_for_cts_();
  this->print_status(status);
}

rev_info_t Si4713Component::get_info_() {
  uint8_t buf[9];  // status byte + 8 bytes of info
  this->read_register(SI4710_CMD_GET_REV, buf, 9);

  // parse the returned data into a rev_info_t struct (skip the status byte)
  rev_info_t revinfo = &buf[1];
  return revinfo;
}

uint8_t Si4713Component::wait_for_cts_() {
  // Poll the status register until the CTS bit is set
  uint8_t status;
  do {
    this->read_register(0x00, &status, 1);
  } while ((status & SI4710_STATUS_CTS) == 0);
  return status;
}
uint8_t Si4713Component::wait_for_cts_(uint8_t status) {
  if ((status & SI4710_STATUS_CTS) == 0) {
    return this->wait_for_cts_();
  }
  return status;
}

// untested
void Si4713Component::set_property(uint16_t property, uint16_t value) {
  uint8_t args[] = {
      0,  // must always be 0
      static_cast<uint8_t>(property >> 8),
      static_cast<uint8_t>(property & 0xFF),
      static_cast<uint8_t>(value >> 8),
      static_cast<uint8_t>(value & 0xFF),
  };
  this->write_register(SI4710_CMD_SET_PROPERTY, args, sizeof(args));

  this->wait_for_cts_();
}

// untested
uint16_t Si4713Component::get_property(uint16_t property) {
  uint8_t args[] = {
      SI4710_CMD_GET_PROPERTY,
      0,  // must always be 0
      static_cast<uint8_t>(property >> 8),
      static_cast<uint8_t>(property & 0xFF),
  };
  uint8_t resp[4];  // status, reserved, value MSB, value LSB
  this->write_read(args, sizeof(args), resp, sizeof(resp));

  return (static_cast<uint16_t>(resp[2]) << 8) | resp[3];
}

// untested
tune_status_t Si4713Component::get_tune_status(bool clear_flags) {
  uint8_t args[] = {
      SI4710_CMD_TX_TUNE_STATUS,
      static_cast<uint8_t>(clear_flags ? 0x1 : 0x0),  // INTACK
  };
  uint8_t resp[7];  // status + 7 bytes of response
  this->write_read(args, sizeof(args), resp, sizeof(resp));

  // parse the returned data into a tune_status_t struct (skip the status byte)
  tune_status_t tunestatus = &resp[1];
  return tunestatus;
}

// untested
asq_status_t Si4713Component::get_asq_status(bool clear_flags) {
  uint8_t args[] = {
      SI4710_CMD_TX_ASQ_STATUS,
      static_cast<uint8_t>(clear_flags ? 0x1 : 0x0),  // INTACK
  };
  uint8_t resp[5];  // status + 4 bytes of response
  this->write_read(args, sizeof(args), resp, sizeof(resp));

  // parse the returned data into an asq_status_t struct (skip the status byte)
  asq_status_t asqstatus = &resp[1];
  return asqstatus;
}

}  // namespace si4713
}  // namespace esphome
