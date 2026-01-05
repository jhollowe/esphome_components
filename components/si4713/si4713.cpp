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
  rev_info_t info = this->get_info();
  this->print_rev_info(info);
  if (info.part_number != 13) {
    ESP_LOGE(TAG, "Device is not Si4713 (part number %u)", info.part_number);
  }

  // TODO remove all below
  // testing setup and hardcoded configuration
  this->measure_freq(9330);
  this->print_tune_status(this->get_tune_status());
  this->set_freq(9330);  // set to 93.3 MHz
  this->print_tune_status(this->get_tune_status());
  // this->set_property(SI4713_PROP_TX_ACOMP_ENABLE, 0b11);  // enable the audio limiter and auto dynamic range control
  this->set_property(SI4713_PROP_TX_LINE_INPUT_LEVEL,
                     0x1000 | 300);  // set line input level to 300 mVPK (attenuation setting 01 (max 301))
  this->set_power(100);
  this->print_tune_status(this->get_tune_status());
  this->set_property(SI4713_PROP_TX_COMPONENT_ENABLE, 0x7);  // Enable pilot, L-R, and RDS

  this->setup_rds(0x27CB, 9);  // program ID KJAH, PTY=9 (top 40)
  uint8_t ps1_1[] = {0x0, 'J', 'H', 'O', 'L'};
  this->write_register(SI4710_CMD_TX_RDS_PS, ps1_1, sizeof(ps1_1));
  uint8_t ps1_2[] = {0x1, 'L', 'O', 'W', 'E'};
  this->write_register(SI4710_CMD_TX_RDS_PS, ps1_2, sizeof(ps1_2));
  this->set_property(SI4713_PROP_TX_RDS_MESSAGE_COUNT, 1);  // 1 PS message
}

void Si4713Component::update() {
  // TODO remove debug print of dynamic statuses
  tune_status_t ts = this->get_tune_status();
  this->print_tune_status(ts);

  asq_status_t asq = this->get_asq_status(1);
  this->print_asq_status(asq);
}

void Si4713Component::reset_() {
  // TODO make this non-blocking with callbacks
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

rev_info_t Si4713Component::get_info() {
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
    ESP_LOGV(TAG, "Checking for CTS...");
    this->read_register(0x00, &status, 1);
  } while ((status & SI4710_STATUS_CTS) == 0);
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
  uint8_t resp[8];  // status + 7 bytes of response
  this->write_read(args, sizeof(args), resp, sizeof(resp));

  // parse the returned data into a tune_status_t struct (keep the status byte)
  tune_status_t tunestatus = resp;
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

  // parse the returned data into an asq_status_t struct (keep the status byte)
  asq_status_t asqstatus = resp;
  return asqstatus;
}

void Si4713Component::set_freq(uint16_t freqKHz) {
  // frequency must be between 7600 and 10800 (76.0 MHz to 108.0 MHz)
  // and in 50 kHz increments. The value is in 10 kHz units.
  ESP_LOGI(TAG, "Tuning to %0.2f MHz", freqKHz / 100.0);
  uint8_t args[] = {
      0,  // reserved
      static_cast<uint8_t>(freqKHz >> 8),
      static_cast<uint8_t>(freqKHz & 0xFF),
  };
  this->write_register(SI4710_CMD_TX_TUNE_FREQ, args, sizeof(args));

  // wait for Wait for the tuning to be done and CTS to be set
  // uint8_t status;
  // do {
  //   status = this->wait_for_cts_();
  // } while ((status & SI4710_STATUS_CTS) == 0);

  this->wait_for_cts_();
}

void Si4713Component::set_power(uint8_t power) {
  // power must be between 88 and 115 (dBµV) or 0 for off.
  ESP_LOGI(TAG, "Setting power to %u dBµV", power);
  uint8_t args[] = {
      0,  // reserved
      0,  // reserved
      power,
      0,  // let the IC choose the antenna capacitance
  };
  this->write_register(SI4710_CMD_TX_TUNE_POWER, args, sizeof(args));

  // wait CTS to be set
  this->wait_for_cts_();
}

void Si4713Component::measure_freq(uint16_t freqKHz) {
  // frequency must be between 7600 and 10800 (76.0 MHz to 108.0 MHz)
  // and in 50 kHz increments. The value is in 10 kHz units.
  ESP_LOGI(TAG, "Measuring Noise on %0.2f MHz", freqKHz / 100.0);
  uint8_t args[] = {
      0,                                     // reserved
      static_cast<uint8_t>(freqKHz >> 8),    // frequency high byte
      static_cast<uint8_t>(freqKHz & 0xFF),  // frequency low byte
      0,                                     // let the IC choose the antenna capacitance
  };
  this->write_register(SI4710_CMD_TX_TUNE_FREQ, args, sizeof(args));

  // wait for the tuning to be done and CTS to be set
  // uint8_t status;
  // do {
  //   status = this->wait_for_cts_();
  // } while ((status & SI4710_STATUS_CTS) == 0);

  this->wait_for_cts_();
}

void Si4713Component::setup_rds(uint16_t programID, uint8_t pty) {
  this->set_property(SI4713_PROP_TX_AUDIO_DEVIATION, 6625);  // 66.25KHz (default is 68.25)
  this->set_property(SI4713_PROP_TX_RDS_DEVIATION, 200);     // 2KHz (default)
  // this->set_property(SI4713_PROP_TX_RDS_INTERRUPT_SOURCE, 0x0001);    // RDS IRQ
  this->set_property(SI4713_PROP_TX_RDS_PI, programID);         // program identifier
  this->set_property(SI4713_PROP_TX_RDS_PS_MIX, 0x03);          // 50% mix (default)
  this->set_property(SI4713_PROP_TX_RDS_PS_REPEAT_COUNT, 3);    // 3 repeats (default)
  this->set_property(SI4713_PROP_TX_RDS_MESSAGE_COUNT, 1);      // 1 message (default)
  this->set_property(SI4713_PROP_TX_RDS_PS_AF, 0xE0E0);         // no AF (default)
  this->set_property(SI4713_PROP_TX_RDS_FIFO_SIZE, 0);          // no FIFO (default)
  this->set_property(SI4713_PROP_TX_COMPONENT_ENABLE, 0x0007);  // enable RDS, stereo, tone
  // 0 PTY is static
  // 0 not compressed
  // 0 not artificial head
  // 1 stereo
  // 0 FIFO and BUFFER not forced to use this PTY
  // 0 not traffic program
  // 00000 PTY Program Type code https://en.wikipedia.org/wiki/Radio_Data_System#Program_types
  // 0 not traffic announcement
  // 1 music
  // 000 reserved
  uint16_t ps_misc = 0b0001000000001000 | ((pty & 0x1F) << 5);
  ESP_LOGD(TAG, "Setting RDS PS Misc to 0x%04X", ps_misc);
  this->set_property(SI4713_PROP_TX_RDS_PS_MISC, ps_misc);
}

void Si4713Component::set_rds_ps(const char *ps) {
  uint8_t len = strlen(ps);
  uint8_t slots = (len + 3) / 4;  // integer division gives the number of slots needed (will be padded)

  // there are 12 buffers each with 8 characters.
  if (len > 96) {
    ESP_LOGE(TAG, "RDS PS can be at most 96 characters");
    return;
  }
  if (len = 0) {
    ESP_LOGW(TAG, "given RDS PS is empty; nothing to set");
    return;
  }

  // TODO actually set the PS string in a loop over the slots

  // TODO set X_RDS_PS_MESSAGE_COUNT
}

void Si4713Component::print_rev_info(rev_info_t info) {
  // clang-format off
    ESP_LOGI(TAG,
      "HW Revision Info:\n"
      "  Part Number: %d\n"
      "  Firmware Version: %u.%u\n"
      "  Patch Version: 0x%04X\n"
      "  Component Version: %u.%u\n"
      "  Chip Revision: 0x%02X",
      info.part_number,
      info.firmware_major, info.firmware_minor,
      info.patch_version,
      info.component_major, info.component_minor,
      info.chip_revision
    );
  // clang-format on
}

void Si4713Component::print_status(uint8_t status) {
  // clang-format off
    ESP_LOGD(TAG,
      "Status Register: 0x%02x (0b" BYTE_TO_BINARY_PATTERN ")\n"
      "  CTS:    %u\n"
      "  ERR:    %u\n"
      "  RDSINT: %u\n"
      "  ASQINT: %u\n"
      "  STCINT: %u\n",
      status, BYTE_TO_BINARY(status),
      status & SI4710_STATUS_CTS ? 1 : 0,
      status & SI4710_STATUS_ERR ? 1 : 0,
      status & SI4710_STATUS_RDSINT ? 1 : 0,
      status & SI4710_STATUS_ASQINT ? 1 : 0,
      status & SI4710_STATUS_STCINT ? 1 : 0);
  // clang-format on
}

void Si4713Component::print_asq_status(asq_status_t asq) {
  // clang-format off
    ESP_LOGD(TAG,
      "ASQ Status:\n"
      "  ASQINT: %u\n"
      "  OVERMOD: %u\n"
      "  IALH: %u\n"
      "  IALL: %u\n"
      "  Input Audio Level: %u dBuV",
      asq.asqint,
      asq.overmod,
      asq.in_audio_detect_high,
      asq.in_audio_detect_low,
      // asq.asqint ? 1 : 0,
      // asq.overmod ? 1 : 0,
      // asq.in_audio_detect_high ? 1 : 0,
      // asq.in_audio_detect_low ? 1 : 0,
      asq.in_audio_level);
  // clang-format on
}

void Si4713Component::print_tune_status(tune_status_t tunestatus) {
  // clang-format off
    ESP_LOGD(TAG,
      "Tune Status:\n"
      "  Frequency: %05u kHz\n"
      "  Power Level: %03u dBµV\n"
      "  Tune Capacitor: %u\n"
      "  Noise Level: %u\n"
      "  STCINT: %u",
      tunestatus.freq,
      tunestatus.power,
      tunestatus.tune_capacitor,
      tunestatus.noise,
      tunestatus.stcint ? 1 : 0);
  // clang-format on
}

}  // namespace si4713
}  // namespace esphome
