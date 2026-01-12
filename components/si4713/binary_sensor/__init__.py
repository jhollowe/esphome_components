# Import required modules
import esphome.codegen as cg
from esphome.components import binary_sensor
import esphome.config_validation as cv
from esphome.const import ENTITY_CATEGORY_DIAGNOSTIC

from .. import CONF_SI4713_ID, DOMAIN, Si4713Hub, Si4713Listener, si4713_ns

DEPENDENCIES = [DOMAIN]


common_classes = [binary_sensor.BinarySensor, Si4713Listener]
Si4713ASQBinarySensor = si4713_ns.class_("Si4713ASQBinarySensor", *common_classes)
Si4713AsqBSType = si4713_ns.enum("Si4713AsqBSType", is_class=True)


CONF_OVERMOD = "overmodulation"
CONF_AUDIO_LOW = "audio_low"
CONF_AUDIO_HIGH = "audio_high"

CONFIG_SCHEMA = cv.Schema(
    {
        cv.GenerateID(CONF_SI4713_ID): cv.use_id(Si4713Hub),
        cv.Optional(CONF_OVERMOD): binary_sensor.binary_sensor_schema(
            Si4713ASQBinarySensor, entity_category=ENTITY_CATEGORY_DIAGNOSTIC
        ),
        cv.Optional(CONF_AUDIO_LOW): binary_sensor.binary_sensor_schema(
            Si4713ASQBinarySensor, entity_category=ENTITY_CATEGORY_DIAGNOSTIC
        ),
        cv.Optional(CONF_AUDIO_HIGH): binary_sensor.binary_sensor_schema(
            Si4713ASQBinarySensor, entity_category=ENTITY_CATEGORY_DIAGNOSTIC
        ),
    }
)


async def to_code(config):
    paren = await cg.get_variable(config[CONF_SI4713_ID])
    if overmod_cfg := config.get(CONF_OVERMOD):
        overmod_sensor = await binary_sensor.new_binary_sensor(overmod_cfg)
        cg.add(overmod_sensor.set_parent(paren))
        cg.add(overmod_sensor.set_type(Si4713AsqBSType.OVERMOD))
        cg.add(paren.register_listener(overmod_sensor))

    if audio_low_cfg := config.get(CONF_AUDIO_LOW):
        audio_low_sensor = await binary_sensor.new_binary_sensor(audio_low_cfg)
        cg.add(audio_low_sensor.set_parent(paren))
        cg.add(audio_low_sensor.set_type(Si4713AsqBSType.AUDIO_LOW))
        cg.add(paren.register_listener(audio_low_sensor))

    if audio_high_cfg := config.get(CONF_AUDIO_HIGH):
        audio_high_sensor = await binary_sensor.new_binary_sensor(audio_high_cfg)
        cg.add(audio_high_sensor.set_parent(paren))
        cg.add(audio_high_sensor.set_type(Si4713AsqBSType.AUDIO_HIGH))
        cg.add(paren.register_listener(audio_high_sensor))
