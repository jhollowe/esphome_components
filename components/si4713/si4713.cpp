#include "si4713.h"

#include "esphome/core/helpers.h"
#include "esphome/core/log.h"

namespace esphome {
namespace si4713 {

static const char *const TAG = "si4713";

void Si4713Component::dump_config() {
  ESP_LOGCONFIG(TAG, "Si4713Component");
  LOG_I2C_DEVICE(this);
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
}

// void Si4713Component::update() {
//   // nothing to do here yet
// }

void Si4713Component::reset_() {
  // reset pin configured so reset before finishing setup
  // RST needs to be pulled low to reset
  if (this->reset_pin_ != nullptr) {
    this->reset_pin_->digital_write(true);
    delay(10);
    this->reset_pin_->digital_write(false);
    delay(10);
    this->reset_pin_->digital_write(true);
  }

  // wait for it to settle
  delay(10);
}

void Si4713Component::power_up_() {
  uint8_t args[] = {
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
  ESP_LOGD(TAG, "Power up status after polling: %b (0x%02x)", status);
}

void Si4713Component::get_info_() {
  // uint8_t buf[9];
  // this->read_register(SI4710_CMD_GET_REV, &buf, 9);

  // // parse the returned data into
  // SI4713Status status = *(SI4713Status*)&buf[0];
  // rev_info_t revinfo = &buf[1];
  // this->pretty_print_rev_info(revinfo);
}

uint8_t Si4713Component::wait_for_cts_() {
  // Poll the status register until the CTS bit is set
  uint8_t status;
  do {
    this->read_register(0x00, &status, 1);
  } while ((status & SI4710_STATUS_CTS) == 0);
  return status;
}

}  // namespace si4713
}  // namespace esphome
