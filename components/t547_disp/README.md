# t547_disp component

This component provides support for the LILYGO T5 4.7" E-Ink display.
It is sourced from https://github.com/nickolay/esphome-lilygo-t547plus/tree/main/components/t547

<!-- TODO also integrate code from https://github.com/Fabian-Schmidt/esphome-lilygo_t5_47_display/tree/main/components/lilygo_t5_47_s3_display -->

Example:
```yaml
display:
  - platform: t547
    # rotation: 180
    update_interval: 30s
    lambda: |-
      it.line(0, 0, 960, 540);
```
