import esphome.codegen as cg
from esphome.components import sensor
import esphome.config_validation as cv
from esphome.const import DEVICE_CLASS_SOUND_PRESSURE, UNIT_DECIBEL

from .. import CONF_SI4713_ID, DOMAIN, Si4713Hub, Si4713Listener, si4713_ns

DEPENDENCIES = [DOMAIN]

common_classes = [sensor.Sensor, cg.Parented.template(Si4713Hub)]
Si4713LineLevelSensor = si4713_ns.class_("Si4713LineLevelSensor", *common_classes, Si4713Listener)

CONF_LINE_LEVEL = "line_level"

CONFIG_SCHEMA = cv.Schema(
    {
        cv.GenerateID(CONF_SI4713_ID): cv.use_id(Si4713Hub),
        cv.Optional(CONF_LINE_LEVEL): sensor.sensor_schema(
            Si4713LineLevelSensor,
            accuracy_decimals=0,
            device_class=DEVICE_CLASS_SOUND_PRESSURE,
            unit_of_measurement=UNIT_DECIBEL,
        ),
    }
)


async def to_code(config):
    paren = await cg.get_variable(config[CONF_SI4713_ID])
    if line_level_cfg := config.get(CONF_LINE_LEVEL):
        line_level_sensor = await sensor.new_sensor(line_level_cfg)
        cg.add(line_level_sensor.set_parent(paren))
        cg.add(paren.register_listener(line_level_sensor))
