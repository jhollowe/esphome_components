#include "si4713.h"

#include "esphome/core/helpers.h"
#include "esphome/core/log.h"

namespace esphome {
namespace si4713 {

// used to detect if a struct has changed
static uint8_t calc_checksum(void *data, size_t size) {
  uint8_t checksum = 0;
  uint8_t *data_bytes = (uint8_t *) data;
  for (size_t i = 0; i < size; i++) {
    checksum ^= data_bytes[i];  // XOR operation
  }
  return checksum;
}

void Si4713Hub::dump_config() {
  ESP_LOGCONFIG(TAG, "Si4713Hub");
  LOG_I2C_DEVICE(this);
  LOG_PIN("  Reset Pin: ", this->reset_pin_);
#ifdef USE_BUTTON
  ESP_LOGCONFIG(TAG, "Buttons:");
  LOG_BUTTON("  ", "TX Reset Button", this->reset_button_);
#endif  // USE_BUTTON
#ifdef USE_SWITCH
  ESP_LOGCONFIG(TAG, "Switches:");
  LOG_SWITCH("  ", "TX Enable Switch", this->enabled_switch_);
  LOG_SWITCH("  ", "Mute Left Switch", this->channel_mute_left_switch_);
  LOG_SWITCH("  ", "Mute Right Switch", this->channel_mute_right_switch_);
  LOG_SWITCH("  ", "Enable Pilot Switch", this->enable_pilot_switch_);
  LOG_SWITCH("  ", "Enable Stereo Switch", this->enable_stereo_switch_);
  LOG_SWITCH("  ", "EnableRDS Switch", this->enable_rds_switch_);
#endif  // USE_SWITCH
#ifdef USE_NUMBER
  ESP_LOGCONFIG(TAG, "Numbers:");
  LOG_NUMBER("  ", "TX Frequency Number", this->frequency_number_);
  LOG_NUMBER("  ", "TX Power Number", this->power_number_);
  LOG_NUMBER("  ", "Max Line Level Number", this->max_line_level_number_);
  LOG_NUMBER("  ", "Low Level Threshold Number", this->low_threshold_number_);
  LOG_NUMBER("  ", "High Level Threshold Number", this->high_threshold_number_);
#endif  // USE_NUMBER
#ifdef USE_BINARY_SENSOR
  ESP_LOGCONFIG(TAG, "Binary Sensors:");
  LOG_BINARY_SENSOR("  ", "Audio High Sensor", this->audio_high_bsensor_);
  LOG_BINARY_SENSOR("  ", "Audio Low Sensor", this->audio_low_bsensor_);
  LOG_BINARY_SENSOR("  ", "Overmod Sensor", this->overmod_bsensor_);
#endif  // USE_BINARY_SENSOR
#ifdef USE_SENSOR
  ESP_LOGCONFIG(TAG, "Sensors:");
  LOG_SENSOR("  ", "Input Line Level Sensor", this->input_line_level_sensor_);
  LOG_SENSOR("  ", "Tune Capacitor Sensor", this->tune_capacitor_sensor_);
#endif  // USE_SENSOR
}

void Si4713Hub::setup() {
  this->reset_pin_->setup();

  this->toggle_reset_pin_();
  this->power_up_();

  // Get hardware info and error if not Si4713
  rev_info_t info = this->get_info();
  this->print_rev_info(info);
  if (info.part_number != 13) {
    ESP_LOGE(TAG, "Device is not Si4713 (part number %u)", info.part_number);
    // TODO error out
  }

  // pull initial data to current state variables
  ESP_LOGV(TAG, "Pulling initial tune and ASQ statuses");
  tune_status_curr_ = this->get_tune_status(true);
  asq_status_curr_ = this->get_asq_status(true);
  tune_status_last_ = tune_status_curr_;
  asq_status_last_ = asq_status_curr_;

  // setup default properties if this is the first setup
  if (!this->has_been_setup_) {
    ESP_LOGV(TAG, "Pulling initial property table");  // DEBUG
    this->get_prop_table_(properties_curr_);
    properties_next_ = properties_curr_;

    ESP_LOGV(TAG, "setting property defaults");  // DEBUG
    properties_next_[SI4713_PROP_TX_LINE_INPUT_LEVEL] = 0x1000 | 300;
    properties_next_[SI4713_PROP_TX_ASQ_LEVEL_LOW] = 0xff & static_cast<int8_t>(-65);  // -65 db (8bit 2's complement)
    properties_next_[SI4713_PROP_TX_ASQ_LEVEL_HIGH] = 0xff & static_cast<int8_t>(-5);  // -5 db (8bit 2's complement)
    properties_next_[SI4713_PROP_TX_ASQ_DURATION_LOW] = 30;                            // 30ms
    properties_next_[SI4713_PROP_TX_COMPONENT_ENABLE] = 0x7;                           // Enable pilot, L-R, and RDS
    this->has_been_setup_ = true;
  } else {
    ESP_LOGV(TAG, "skipping property defaults");  // DEBUG
    // sets the "current" properties to 0 to force re-application of all next properties
    for (auto &[prop, val] : properties_curr_) {
      val = 0;
    }
  }
  this->print_prop_table(properties_curr_);  // DEBUG

  this->print_tune_status(this->get_tune_status());  // DEBUG
  this->set_freq(frequency_);
  this->set_power(power_);
  this->print_tune_status(this->get_tune_status());  // DEBUG

  this->setup_rds(prg_id_, pty_);  // program ID KJAH, PTY=9 (top 40)
  this->set_ps(ps_buffer_);
}

void Si4713Hub::update() {
  tune_status_last_ = tune_status_curr_;
  asq_status_last_ = asq_status_curr_;

  tune_status_curr_ = this->get_tune_status(true);
  asq_status_curr_ = this->get_asq_status(true);

  // TODO is this less efficient than always sending the
  //      status to the listener and letting it do a diff check?
  if (calc_checksum(&tune_status_last_, sizeof(tune_status_t)) !=
      calc_checksum(&tune_status_curr_, sizeof(tune_status_t))) {
    // notify listeners of tune status change
    ESP_LOGV(TAG, "notifying listeners of tune status change");
    for (auto &listener : this->listeners_) {
      listener->on_tune_status(tune_status_curr_);
    }
    // update stored power and frequency (prevent overwriting power if disabled (power==0))
    power_ = tune_status_curr_.power == 0 ? power_ : tune_status_curr_.power;
    frequency_ = tune_status_curr_.freq;
  }
  if (calc_checksum(&asq_status_last_, sizeof(asq_status_t)) !=
      calc_checksum(&asq_status_curr_, sizeof(asq_status_t))) {
    // notify listeners of ASQ status change
    ESP_LOGV(TAG, "notifying listeners of ASQ status change");
    for (auto &listener : this->listeners_) {
      listener->on_asq_status(asq_status_curr_);
    }
  }

  // this->print_asq_status(asq_status_curr_);    // DEBUG
  // this->print_tune_status(tune_status_curr_);  // DEBUG

  // iterate through next properties and notify listeners if the value is different from current
  for (const auto &[prop, val] : properties_next_) {
    if (val != properties_curr_[prop]) {
      ESP_LOGV(TAG, "Property 0x%04X changed from 0x%04x to 0x%04x", prop, properties_curr_[prop], val);
      // set the new property value on the device
      this->set_property_(prop, val);
      // notify listeners of property change
      for (auto &listener : this->listeners_) {
        listener->on_property(prop, val);
      }
      // update current property value
      properties_curr_[prop] = val;
    }
  }

  // DEBUG make sure the props are getting set correctly
  // prop_table_t real_props = properties_curr_;
  // this->get_prop_table_(real_props);
  // this->print_prop_table(real_props);
}

void Si4713Hub::toggle_reset_pin_() {
  // RST needs to be pulled low to reset
  this->reset_pin_->digital_write(true);
  delay(10);
  this->reset_pin_->digital_write(false);
  delay(10);
  this->reset_pin_->digital_write(true);
}

void Si4713Hub::power_up_() {
  const uint8_t args[] = {
      // 0 CTS interrupt disabled
      // 0 GPO2 output disabled
      // 0 Boot normally (no firmware patch)
      // 1 crystal oscillator Enabled
      // 0010 function: FM transmit
      0b00010010,
      // analog input mode
      0x50,
      // OR digital input mode
      // 0x0f
  };
  uint8_t status;
  ESP_LOGV(TAG, "Powering up Si4713...");
  this->write_register(SI4710_CMD_POWER_UP, args, sizeof(args));
  status = this->wait_for_cts_();
  this->print_status(status);
}

rev_info_t Si4713Hub::get_info() {
  uint8_t buf[9];  // status byte + 8 bytes of info
  this->read_register(SI4710_CMD_GET_REV, buf, 9);

  // parse the returned data into a rev_info_t struct (skip the status byte)
  rev_info_t revinfo = &buf[1];
  return revinfo;
}

uint8_t Si4713Hub::wait_for_cts_() {
  // Poll the status register until the CTS bit is set
  uint8_t status;
  size_t max_attempts = 20;
  do {
    ESP_LOGV(TAG, "Checking for CTS...");
    err_ = this->read_register(0x00, &status, 1);
    if (err_ != i2c::ErrorCode::NO_ERROR) {
      ESP_LOGE(TAG, "I2C error while waiting for CTS: %d", static_cast<int>(err_));
    }
    // TODO should this use SI4710_CMD_GET_INT_STATUS to get the status?
    max_attempts--;
  } while ((status & SI4710_STATUS_CTS) == 0 && max_attempts > 0);
  if (max_attempts == 0) {
    // TODO error out
    this->status_set_error(LOG_STR("Timed out waiting for Clear To Send (CTS) from Si4713"));
  }
  return status;
}

// allow passing in an existing status byte to sort-circuit if previous command returned a status byte with CTS
uint8_t Si4713Hub::wait_for_cts_(uint8_t status) {
  if ((status & SI4710_STATUS_CTS) == 0) {
    return this->wait_for_cts_();
  }
  return status;
}

void Si4713Hub::set_property_(uint16_t property, uint16_t value) {
  uint8_t args[] = {
      0,  // reserved
      static_cast<uint8_t>(property >> 8),
      static_cast<uint8_t>(property & 0xFF),
      static_cast<uint8_t>(value >> 8),
      static_cast<uint8_t>(value & 0xFF),
  };
  this->write_register(SI4710_CMD_SET_PROPERTY, args, sizeof(args));

  this->wait_for_cts_();
}

uint16_t Si4713Hub::get_property(uint16_t property) {
  uint8_t args[] = {
      SI4710_CMD_GET_PROPERTY,
      0,  // reserved
      static_cast<uint8_t>(property >> 8),
      static_cast<uint8_t>(property & 0xFF),
  };
  uint8_t resp[4];  // status, reserved, value MSB, value LSB
  this->write_read(args, sizeof(args), resp, sizeof(resp));

  return (static_cast<uint16_t>(resp[2]) << 8) | resp[3];
}

tune_status_t Si4713Hub::get_tune_status(bool clear_flags) {
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

asq_status_t Si4713Hub::get_asq_status(bool clear_flags) {
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

void Si4713Hub::get_prop_table_(prop_table_t &table) {
  // read all tracked properties in the table and update their values
  for (auto &[prop, val] : table) {
    val = this->get_property(prop);
  }
}

void Si4713Hub::set_freq(uint16_t freqKHz) {
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

void Si4713Hub::set_enabled(bool enabled) {
  this->enabled_ = enabled;
  if (enabled) {
    ESP_LOGI(TAG, "Transmitter enabled; applying stored power level of %u dBµV", this->power_);
    this->set_power_direct_(this->power_);
  } else {
    ESP_LOGI(TAG, "Transmitter disabled");
    this->set_power_direct_(0);  // set power to 0 to disable
  }
}

void Si4713Hub::set_power(uint8_t power) {
  // store the desired power level
  this->power_ = power;

  if (this->enabled_) {
    this->set_power_direct_(power);
  } else {
    ESP_LOGI(TAG, "Power setting of %u dBµV stored but not applied since transmitter is disabled", power);
  }
}

void Si4713Hub::set_power_direct_(uint8_t power) {
  // power must be between 88 and ~115 (dBµV) or 0 for off.
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

void Si4713Hub::setup_rds(uint16_t programID, uint8_t pty) {
  this->set_property(SI4713_PROP_TX_AUDIO_DEVIATION, 6625);  // 66.25KHz (default is 68.25)
  this->set_property(SI4713_PROP_TX_RDS_DEVIATION, 200);     // 2KHz (default)
  // this->set_property(SI4713_PROP_TX_RDS_INTERRUPT_SOURCE, 0x0001);    // RDS IRQ
  this->set_property(SI4713_PROP_TX_RDS_PI, programID);       // program identifier
  this->set_property(SI4713_PROP_TX_RDS_PS_MIX, 0x03);        // 50% mix (default)
  this->set_property(SI4713_PROP_TX_RDS_PS_REPEAT_COUNT, 3);  // 3 repeats (default)
  // this->set_property(SI4713_PROP_TX_RDS_MESSAGE_COUNT, 1);    // 1 message (default)
  this->set_property(SI4713_PROP_TX_RDS_PS_AF, 0xE0E0);  // no AF (default)
  this->set_property(SI4713_PROP_TX_RDS_FIFO_SIZE, 4);   // 4 blocks (3 for a message plus 1 required padding)
  // 0 PTY is static
  // 0 not compressed
  // 0 not artificial head
  // 1 stereo
  // 1 FIFO and BUFFER are forced to use the following TP/PTY
  //        (hardware injects them into any RDS group in the FIFO/buffer)
  // 0 not traffic program
  // 00000 PTY Program Type code https://en.wikipedia.org/wiki/Radio_Data_System#Program_types
  // 0 not traffic announcement
  // 1 music
  // 000 reserved
  uint16_t ps_misc = 0b0001100000001000 | ((pty & 0x1F) << 5);
  ESP_LOGD(TAG, "Setting RDS PS Misc to 0x%04X", ps_misc);
  this->set_property(SI4713_PROP_TX_RDS_PS_MISC, ps_misc);
}

void Si4713Hub::clear_and_write_rds(const std::vector<uint16_t> &buffer, bool is_fifo) {
  const char *type_str = is_fifo ? "FIFO" : "buffer";
  if (buffer.size() % 3 != 0) {
    ESP_LOGE(TAG, "RDS %s must contain groups of 3 16-bit blocks (2, 3, 4), but got %u", type_str, buffer.size());
    return;
  }
  uint8_t resp[6];  // status, flags, cbuff avail, cbuff used, fifo avail, fifo used

  // if the buffer is empty, just send a clear command
  if (buffer.empty()) {
    uint8_t args[] = {
        SI4710_CMD_TX_RDS_BUFF,
        // 0 send to circular buffer, 1 send to FIFO
        // 0000 reserved
        // 0 load into the buffer
        // 1 clear the buffer
        // 0 don't clear the interrupt
        static_cast<uint8_t>(0b00000100 | (is_fifo ? 0b10000000 : 0x00)),
        0,  // the rest of the bytes are ignored when clearing
        0,
        0,
        0,
        0,
    };
    this->write_read(args, sizeof(args), resp, sizeof(resp));
    ESP_LOGV(TAG, "Cleared RDS %s (buffer %u/%u, FIFO %u/%u)", type_str, resp[3], resp[2] + resp[3], resp[5],
             resp[4] + resp[5]);
    this->wait_for_cts_(resp[0]);
    return;
  }

  // iterate over the buffer in groups of 3 16-bit blocks (Blocks 2, 3, 4)
  for (size_t i = 0; i < buffer.size(); i += 3) {
    uint16_t block2 = buffer[i];
    uint16_t block3 = buffer[i + 1];
    uint16_t block4 = buffer[i + 2];

    uint8_t args[] = {
        SI4710_CMD_TX_RDS_BUFF,
        // 0 send to circular buffer, 1 send to FIFO
        // 0000 reserved
        // 1 load into the buffer
        // 0/1 clear the buffer (if i==0)
        // 0 don't clear the interrupt
        static_cast<uint8_t>(0b00000100 | (is_fifo ? 0b10000000 : 0x00) | (i == 0 ? 0b00000010 : 0x00)),
        // 16 bits of block B
        static_cast<uint8_t>(block2 >> 8),
        static_cast<uint8_t>(block2 & 0xFF),
        // 16 bits of block C
        static_cast<uint8_t>(block3 >> 8),
        static_cast<uint8_t>(block3 & 0xFF),
        // 16 bits of block D
        static_cast<uint8_t>(block4 >> 8),
        static_cast<uint8_t>(block4 & 0xFF),
    };
    this->write_read(args, sizeof(args), resp, sizeof(resp));
    ESP_LOGV(TAG, "Set RDS %s groups %u-%u (buffer %u/%u, FIFO %u/%u)", type_str, i, i + 2, resp[3], resp[2] + resp[3],
             resp[5], resp[4] + resp[5]);
    this->wait_for_cts_(resp[0]);
  }
}

std::vector<uint16_t> Si4713Hub::generate_radio_text_bytes(const char *s, bool ab_flag) {
  std::vector<uint16_t> blocks = {};
  size_t len = strlen(s);
  size_t segments = (len + 3) / 4;  // integer division gives the number of segments needed (will be padded)

  if (len > 64) {
    ESP_LOGW(TAG, "RDS Radio Text is limited to 64 characters, but got %u. Truncating.", len);
    len = 64;
    segments = 16;
  }

  ESP_LOGD(TAG, "Creating RDS Type 2A blocks from string '%s' requiring %u segments", s, segments);

  for (uint8_t i = 0; i < segments; i++) {
    // https://en.wikipedia.org/wiki/Radio_Data_System#Group_type_2_%E2%80%93_Radio_text
    // 0010 group type 2 (Radio Text)
    // 0 version A
    // x traffic program (injected due to PS_MISC setting)
    // xxxxx PTY (injected due to PS_MISC setting)
    // y A/B toggle (used to tell receiver to clear the buffer and update to new text)
    // zzzz text chunk index
    uint16_t block2 = 0b0010000000000000 | (0x0F & i);
    if (ab_flag) {
      block2 |= 0b00010000;  // set A/B flag
    }
    // block3 contains char1 and char2, block4 contains char3 and char4
    uint8_t c1 = (i * 4 < len) ? static_cast<uint8_t>(s[i * 4]) : ' ';
    uint8_t c2 = (i * 4 + 1 < len) ? static_cast<uint8_t>(s[i * 4 + 1]) : ' ';
    uint8_t c3 = (i * 4 + 2 < len) ? static_cast<uint8_t>(s[i * 4 + 2]) : ' ';
    uint8_t c4 = (i * 4 + 3 < len) ? static_cast<uint8_t>(s[i * 4 + 3]) : ' ';

    uint16_t block3 = (static_cast<uint16_t>(c1) << 8) | static_cast<uint16_t>(c2);
    uint16_t block4 = (static_cast<uint16_t>(c3) << 8) | static_cast<uint16_t>(c4);

    blocks.push_back(block2);
    blocks.push_back(block3);
    blocks.push_back(block4);
  }

  return blocks;
}

void Si4713Hub::set_ps(std::string ps) {
  // PS is groups of 8 characters, sent chunks of 4 characters
  // Radios only show 8 characters at a time, but the Si4713 will rotate through up to 11 PS's
  uint8_t len = ps.length();

  if (len > 88) {
    ESP_LOGW(TAG, "RDS Program Service (PS) is limited to 88 characters, but got %u. Truncating.", len);
    ps = ps.substr(0, 88);
    len = 88;
  }

  uint8_t slots = (len + 3) / 4;
  uint8_t resp[1];

  for (uint8_t i = 0; i < slots; i++) {
    uint8_t args[] = {
        SI4710_CMD_TX_RDS_PS,
        // 5 bits of PS segment index (0: first 4 chars or PS0, 1: second 4 chars or PS1, etc.)
        static_cast<uint8_t>(i & 0x1F),
        // 4 chars of PS data (padded with spaces if not full)
        static_cast<uint8_t>(i * 4 < len) ? ps[i * 4] : ' ',
        static_cast<uint8_t>(i * 4 + 1 < len) ? ps[i * 4 + 1] : ' ',
        static_cast<uint8_t>(i * 4 + 2 < len) ? ps[i * 4 + 2] : ' ',
        static_cast<uint8_t>(i * 4 + 3 < len) ? ps[i * 4 + 3] : ' ',
    };
    // this->write_register(SI4710_CMD_TX_RDS_PS, args, sizeof(args));
    this->write_read(args, sizeof(args), resp, sizeof(resp));
  }

  // num of PS messages is half the number of slots (rounded up)
  this->set_property(SI4713_PROP_TX_RDS_MESSAGE_COUNT, slots / 2 + slots % 2);
}

std::vector<uint16_t> Si4713Hub::generate_timestamp_bytes(ESPTime utc_time) {
  ESP_LOGD(TAG, "time in generate_timestamp_bytes: %s with offset %d", utc_time.strftime("%Y-%m-%d %H:%M:%S").c_str(),
           utc_time.timezone_offset());

  // Modified Julian Date (17 bits)
  // Calculate MJD from year/month/day using the Julian Day Number algorithm.
  // MJD = JDN - 2400001, where JDN is the Julian Day Number for the date.
  uint32_t mjd;
  {
    int y = static_cast<int>(utc_time.year);
    int m = static_cast<int>(utc_time.month);
    int d = static_cast<int>(utc_time.day_of_month);
    int a = (14 - m) / 12;
    int y2 = y + 4800 - a;
    int m2 = m + 12 * a - 3;
    int jdn = d + (153 * m2 + 2) / 5 + 365 * y2 + y2 / 4 - y2 / 100 + y2 / 400 - 32045;
    mjd = static_cast<uint32_t>(jdn - 2400001);
  }

  uint8_t utc_hour = utc_time.hour;      // 5 bits (0-23)
  uint8_t utc_minute = utc_time.minute;  // 6 bits (0-59)

  // 5 bits (in 30-minute increments, range -15.5 to +15.5h)
  uint8_t local_offset = static_cast<uint8_t>(std::abs(utc_time.timezone_offset()) / 1800);
  // 1 bit sign (0 = + (east of UTC), 1 = - (west of UTC))
  bool local_offset_negative = utc_time.timezone_offset() < 0;

  ESP_LOGD(TAG, "mjd: 0x%05X, hours: 0x%02X, minutes: 0x%02X, local_offset: 0x%02X, local_offset_negative: %u", mjd,
           utc_hour, utc_minute, local_offset, local_offset_negative);

  std::vector<uint16_t> blocks = {
      // https://en.wikipedia.org/wiki/Radio_Data_System#Group_type_4_%E2%80%93_Version_A_%E2%80%93_Clock_time_and_date
      // 0100 group type 4 (Clock/Time)
      // 0 version A
      // x traffic program (injected due to PS_MISC setting)
      // xxxxx PTY (injected due to PS_MISC setting)
      // 000 reserved
      // yy first 2 bits of mjd
      0b0100000000000000 | ((mjd >> 15) & 0x3),
      // bits 0-14 of mjd then 1 bit of hour
      ((mjd & 0x7FFF) << 1) | ((utc_hour >> 4) & 0x1),
      // hhh bits 0-3 of hour
      // mmmmmm minute
      // s local offset sign
      // lllll local offset
      ((utc_hour & 0xF) << 12) | ((utc_minute & 0x3F) << 6) | ((local_offset_negative ? 1 : 0) << 5) |
          (local_offset & 0x1F),
  };

  // dump the blocks as hex
  ESP_LOGD(TAG, "Generated timestamp bytes: 0x%04X 0x%04X 0x%04X", blocks[0], blocks[1], blocks[2]);

  return blocks;
}

void Si4713Hub::print_rev_info(const rev_info_t &info) {
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

void Si4713Hub::print_status(uint8_t status) {
  // clang-format off
    ESP_LOGV(TAG,
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

void Si4713Hub::print_asq_status(const asq_status_t &asq) {
  // clang-format off
    ESP_LOGD(TAG,
      "ASQ Status:\n"
      "  ASQINT: %u\n"
      "  OVERMOD: %u\n"
      "  IALH: %u\n"
      "  IALL: %u\n"
      "  Input Audio Level: %i dBfs",
      asq.asqint,
      asq.overmod,
      asq.in_audio_detect_high,
      asq.in_audio_detect_low,
      asq.in_audio_level);
  // clang-format on
}

void Si4713Hub::print_tune_status(const tune_status_t &tunestatus) {
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

void Si4713Hub::print_prop_table(const prop_table_t &table) {
  ESP_LOGD(TAG, "Si4713 Properties:");
  for (const auto &[prop, val] : table) {
    ESP_LOGD(TAG, "  Property 0x%04x: 0x%04x", prop, val);
  }
}

}  // namespace si4713
}  // namespace esphome
