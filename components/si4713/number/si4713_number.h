#pragma once

#include "esphome/core/component.h"
#include "esphome/components/number/number.h"
#include "esphome/components/si4713/si4713.h"

namespace esphome {
namespace si4713 {

class Si4713BaseNumber : public number::Number, public Si4713Listener {};

class Si4713FrequencyNumber : public Si4713BaseNumber {
  // overrides from interface (Number)
  void control(float value) override;
  // overrides from interface (Si4713Listener)
  void on_tune_status(const tune_status_t &status) override;
};

class Si4713PowerNumber : public Si4713BaseNumber {
  // overrides from interface (Number)
  void control(float value) override;
  // overrides from interface (Si4713Listener)
  void on_tune_status(const tune_status_t &status) override;
};

class Si4713MaxLineLevelNumber : public Si4713BaseNumber {
  // overrides from interface (Number)
  void control(float value) override;
  // overrides from interface (Si4713Listener)
  void on_property(uint16_t reg, uint16_t value) override;
};

class Si4713LevelThresholdNumber : public Si4713BaseNumber {
 public:
  void set_is_low_thresh(bool is_low) { this->is_low_thresh_ = is_low; }
  bool get_is_low_thresh() const { return this->is_low_thresh_; }
  // overrides from interface (Number)
  void control(float value) override;
  // overrides from interface (Si4713Listener)
  void on_property(uint16_t reg, uint16_t value) override;

 private:
  bool is_low_thresh_;
};

}  // namespace si4713
}  // namespace esphome
