#include "si4713.h"

#include "esphome/core/helpers.h"
#include "esphome/core/log.h"

namespace esphome {
namespace si4713 {

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

void Si4713Component::update() {
  // nothing to do here yet
}

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

// void Si4713Component::power_down_() {
//   uint8_t status;
//   ESP_LOGV(TAG, "Powering down Si4713...");
//   this->write_register(SI4710_CMD_POWER_DOWN, nullptr, 0);
//   status = this->wait_for_cts_();
//   this->print_status(status);
// }

rev_info_t Si4713Component::get_info_() {
  uint8_t buf[9];  // status byte + 8 bytes of info
  this->read_register(SI4710_CMD_GET_REV, buf, 9);

  // parse the returned data into a rev_info_t struct (skip the status byte)
  rev_info_t revinfo = &buf[1];
  this->pretty_print_rev_info(revinfo);

  // pass the existing status byte, if it indicates CTS, we don't have to query at all
  this->wait_for_cts_(buf[0]);
  return &(buf[1]);
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

}  // namespace si4713
}  // namespace esphome
