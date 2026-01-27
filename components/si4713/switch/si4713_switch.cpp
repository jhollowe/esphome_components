#include "si4713_switch.h"

namespace esphome {
namespace si4713 {

void Si4713EnableSwitch::write_state(bool state) {
  if (parent_ != nullptr) {
    parent_->set_enabled(state);
    publish_state(state);
  }
}

void Si4713EnableSwitch::on_tune_status(const tune_status_t &status) {
  ESP_LOGD(TAG, "Si4713EnableSwitch received tune status update: power=%u", status.power);
  if (parent_ != nullptr) {
    bool is_enabled = status.power > 0;
    if (state != is_enabled)
      publish_state(is_enabled);
  }
}

void Si4713ChannelMuteSwitch::write_state(bool state) {
  if (parent_ != nullptr) {
    // Left channel is bit 1, Right channel is bit 0
    // 0 is unmuted, 1 is muted

    // clear the changing bit
    (*(parent_->get_properties_next()))[SI4713_PROP_TX_LINE_INPUT_MUTE] &= ~(1 << (is_left_channel_ ? 1 : 0));
    // set the changing bit
    (*(parent_->get_properties_next()))[SI4713_PROP_TX_LINE_INPUT_MUTE] |=
        (static_cast<uint16_t>(state) << (is_left_channel_ ? 1 : 0));
  }
}

void Si4713ChannelMuteSwitch::on_property(uint16_t reg, uint16_t value) {
  if (reg == SI4713_PROP_TX_LINE_INPUT_MUTE) {
    bool is_muted = (value >> (is_left_channel_ ? 1 : 0)) & 0x1;
    publish_state(is_muted);
  }
}

void Si4713ComponentSwitch::write_state(bool state) {
  if (parent_ != nullptr) {
    if (state) {
      // set the bit
      (*(parent_->get_properties_next()))[SI4713_PROP_TX_COMPONENT_ENABLE] |= (1 << bit_pos_);
    } else {
      // clear the bit
      (*(parent_->get_properties_next()))[SI4713_PROP_TX_COMPONENT_ENABLE] &= ~(1 << bit_pos_);
    }
  }
}

void Si4713ComponentSwitch::on_property(uint16_t reg, uint16_t value) {
  if (reg == SI4713_PROP_TX_COMPONENT_ENABLE) {
    bool is_enabled = (value >> bit_pos_) & 0x1;
    publish_state(is_enabled);
  }
}

}  // namespace si4713
}  // namespace esphome
