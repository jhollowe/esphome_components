# Si4713 component

This component provides support for the Si4713 FM transmitter chip

Adafruit library: https://github.com/adafruit/Adafruit-Si4713-Library
Datasheet: https://cdn-shop.adafruit.com/datasheets/Si4712-13-B30.pdf
Programming Guide: https://cdn-shop.adafruit.com/datasheets/SiLabs%20Programming%20guide%20AN332.pdf

Example:
```yaml
i2c:
  sda: GPIO18
  scl: GPIO19
  scan: true
  id: fm_tx_bus 

si4713:
  TODO: asdf
```
