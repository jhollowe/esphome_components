# Si4713 component

This component provides support for the Si4713 FM transmitter chip

## Key Features

- adjustable transmit frequency and power
- adjustable input sensitivity
- configurable high/low input level indicators (great for automation triggers)
- RDS/RDMS support (WIP)

## Documentation

Adafruit library: https://github.com/adafruit/Adafruit-Si4713-Library
Datasheet: https://cdn-shop.adafruit.com/datasheets/Si4712-13-B30.pdf
Programming Guide: https://cdn-shop.adafruit.com/datasheets/SiLabs%20Programming%20guide%20AN332.pdf

## Example

see [si4713.yaml](../../example_configs/si4713.yaml) for a full example

### Minimal Configuration
```yaml
i2c:
  id: fm_tx_bus
  sda: GPIO14
  scl: GPIO13
  scan: false

si4713:
  id: tx_id
  reset_pin: GPIO12
```

### Automatic Enable/Disable based on input audio level
```yaml
button:
  - platform: si4713
    reset:
      id: tx_reset

binary_sensor:
  - platform: si4713
    audio_low:
      id: tx_audio_low
      on_press:
        # If tx_audio_low stays ON for 30s, turn OFF tx_enable
        - delay: 30s
        - if:
            condition:
              binary_sensor.is_on: tx_audio_low
            then:
              - switch.turn_off: tx_enable
      on_release:
        # As soon as tx_audio_low turns OFF, turn ON tx_enable
        - switch.turn_on: tx_enable

number:
  - platform: si4713
    low_threshold:
      name: "Audio Low Threshold"
      id: tx_asq_low_threshold
```
