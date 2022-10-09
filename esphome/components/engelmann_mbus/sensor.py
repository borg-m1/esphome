import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.components import sensor, uart
from esphome.const import (
    CONF_ID,
    CONF_POWER,
    DEVICE_CLASS_TEMPERATURE,
    DEVICE_CLASS_POWER,
    STATE_CLASS_MEASUREMENT,
    UNIT_CELSIUS,
    UNIT_KELVIN,
    UNIT_WATT,
)

DEPENDENCIES = ["uart"]

engelmann_mbus_ns = cg.esphome_ns.namespace("engelmannMBus")
Engelmann_mbus = engelmann_mbus_ns.class_(
    "EngelmannMBus", cg.PollingComponent, uart.UARTDevice
)

CONF_TEMPERATURE_A = "temperature_a"
CONF_TEMPERATURE_B = "temperature_b"
CONF_TEMPERATURE_C = "temperature_c"

CONFIG_SCHEMA = (
    cv.Schema(
        {
            cv.GenerateID(): cv.declare_id(Engelmann_mbus),
            cv.Optional(CONF_TEMPERATURE_A): sensor.sensor_schema(
                unit_of_measurement=UNIT_CELSIUS,
                accuracy_decimals=2,
                device_class=DEVICE_CLASS_TEMPERATURE,
                state_class=STATE_CLASS_MEASUREMENT,
            ),
            cv.Optional(CONF_TEMPERATURE_B): sensor.sensor_schema(
                unit_of_measurement=UNIT_CELSIUS,
                accuracy_decimals=2,
                device_class=DEVICE_CLASS_TEMPERATURE,
                state_class=STATE_CLASS_MEASUREMENT,
            ),
            cv.Optional(CONF_TEMPERATURE_C): sensor.sensor_schema(
                unit_of_measurement=UNIT_KELVIN,
                accuracy_decimals=2,
                device_class=DEVICE_CLASS_TEMPERATURE,
                state_class=STATE_CLASS_MEASUREMENT,
            ),
            cv.Optional(CONF_POWER): sensor.sensor_schema(
                unit_of_measurement=UNIT_WATT,
                accuracy_decimals=2,
                device_class=DEVICE_CLASS_POWER,
                state_class=STATE_CLASS_MEASUREMENT,
            ),
        }
    )
    .extend(cv.polling_component_schema("60s"))
    .extend(uart.UART_DEVICE_SCHEMA)
)


async def to_code(config):
    var = cg.new_Pvariable(config[CONF_ID])
    await cg.register_component(var, config)
    await uart.register_uart_device(var, config)

    if CONF_TEMPERATURE_A in config:
        conf = config[CONF_TEMPERATURE_A]
        sens = await sensor.new_sensor(conf)
        cg.add(var.set_flow_temperature_sensor(sens))
    if CONF_TEMPERATURE_B in config:
        conf = config[CONF_TEMPERATURE_B]
        sens = await sensor.new_sensor(conf)
        cg.add(var.set_return_temperature_sensor(sens))
    if CONF_TEMPERATURE_C in config:
        conf = config[CONF_TEMPERATURE_C]
        sens = await sensor.new_sensor(conf)
        cg.add(var.set_difference_temperature_sensor(sens))
    if CONF_POWER in config:
        conf = config[CONF_POWER]
        sens = await sensor.new_sensor(conf)
        cg.add(var.set_power_sensor(sens))
