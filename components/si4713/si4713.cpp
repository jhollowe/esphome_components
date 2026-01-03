#include "si4713.h"

#include "esphome/core/helpers.h"
#include "esphome/core/log.h"

namespace esphome {
namespace si4713 {

static const char *const TAG = "si4713";

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
  this->get_info_();
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
  ESP_LOGD(TAG, "Power up status after polling: 0x%02x (0b" BYTE_TO_BINARY_PATTERN ")", status, BYTE_TO_BINARY(status));
}

void Si4713Component::get_info_() {
  uint8_t buf[9];
  this->read_register(SI4710_CMD_GET_REV, buf, 9);

  ESP_LOGI("si4713.revinfo", "Part Number: (si47)%u", buf[1]);
  ESP_LOGI("si4713.revinfo", "Firmware Version: %u.%u", buf[2], buf[3]);
  ESP_LOGI("si4713.revinfo", "Patch Version: 0x%04X", (static_cast<uint16_t>(buf[4]) << 8) | buf[5]);
  ESP_LOGI("si4713.revinfo", "Component Version: %u.%u", buf[6], buf[7]);
  ESP_LOGI("si4713.revinfo", "Chip Revision: 0x%02X", buf[8]);

  // parse the returned data into
  // SI4713Status status = *(SI4713Status *) &buf[0];
  // rev_info_t revinfo = &buf[1];
  // this->pretty_print_rev_info(revinfo);
  this->print_status(buf[0]);
  buf[0] = this->wait_for_cts_();
  this->print_status(buf[0]);
}

void Si4713Component::print_status(uint8_t status) {
  ESP_LOGD(TAG, "Status Register: 0x%02X", status);
  ESP_LOGD(TAG, "  STCINT: %u", (status >> 0) & 0x01);
  ESP_LOGD(TAG, "  ASQINT: %u", (status >> 1) & 0x01);
  ESP_LOGD(TAG, "  RDSINT: %u", (status >> 2) & 0x01);
  ESP_LOGD(TAG, "  ERR: %u", (status >> 6) & 0x01);
  ESP_LOGD(TAG, "  CTS: %u", (status >> 7) & 0x01);
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
