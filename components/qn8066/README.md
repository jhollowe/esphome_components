# Si4713 component

This component provides support for the QN8066 FM Transceiver chip
Initial support is only focused on transmitting.

## Key Features

- adjustable transmit frequency
- RDS/RDMS support

## Documentation

Arduino library: https://github.com/pu2clr/QN8066
Datasheet: https://vrtp.ru/index.php?act=Attach&type=post&id=913773

## Example

see [qn8066.yaml](../../example_configs/qn8066.yaml) for a full example

### Minimal Configuration
```yaml
i2c:
  id: fm_tx_bus
  sda: GPIO14
  scl: GPIO13
  scan: false

qn8066:
  id: tx_id
```
