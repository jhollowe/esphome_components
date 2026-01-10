#pragma once

#include "esphome/core/component.h"
#include "esphome/components/button/button.h"
#include "esphome/components/si4713/si4713.h"

namespace esphome {
namespace si4713 {

const char *const B_TAG = "si4713.button";

class Si4713BaseButton : public button::Button, public Parented<Si4713Hub> {};

class Si4713ResetButton : public Si4713BaseButton {
  // overrides from interface (Button)
  void press_action() override {
    ESP_LOGD(B_TAG, "Resetting Si4713 via button press");
    this->get_parent()->setup();
  };
};

}  // namespace si4713
}  // namespace esphome
