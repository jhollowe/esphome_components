# t547_disp component

This component provides support for the LILYGO T5 4.7" E-Ink display.
It is sourced from https://github.com/nickolay/esphome-lilygo-t547plus/tree/main/components/t547

Example:
```yaml
display:
  - platform: t547
    # rotation: 180
    update_interval: 30s
    lambda: |-
      it.line(0, 0, 960, 540);
```
