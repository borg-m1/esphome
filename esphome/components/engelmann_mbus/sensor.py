import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.components import sensor, uart
from esphome.const import (
    CONF_ID,
    CONF_POWER,
    CONF_ENERGY,
    DEVICE_CLASS_ENERGY,
    STATE_CLASS_MEASUREMENT,
    UNIT_CELSIUS,
    UNIT_KELVIN,
    UNIT_WATT,
    UNIT_CUBIC_METER,
    UNIT_KILOWATT_HOURS,
)

DEPENDENCIES = ["uart"]

engelmann_mbus_ns = cg.esphome_ns.namespace("engelmannMBus")
Engelmann_mbus = engelmann_mbus_ns.class_(
    "EngelmannMBus", cg.PollingComponent, uart.UARTDevice
)

CONF_TEMPERATURE_A = "flow_temperature"
CONF_TEMPERATURE_B = "return_temperature"
CONF_TEMPERATURE_C = "difference_temperature"

CONF_FLOW_RATE = "volume_flow_rate"

CONF_VOLUME = "volume"

CONFIG_SCHEMA = (
    cv.Schema(
        {
            cv.GenerateID(): cv.declare_id(Engelmann_mbus),
            cv.Optional(CONF_TEMPERATURE_A): sensor.sensor_schema(
                unit_of_measurement=UNIT_CELSIUS,
                accuracy_decimals=0,
                device_class=DEVICE_CLASS_ENERGY,
                state_class=STATE_CLASS_MEASUREMENT,
            ),
            cv.Optional(CONF_TEMPERATURE_B): sensor.sensor_schema(
                unit_of_measurement=UNIT_CELSIUS,
                accuracy_decimals=0,
                device_class=DEVICE_CLASS_ENERGY,
                state_class=STATE_CLASS_MEASUREMENT,
            ),
            cv.Optional(CONF_TEMPERATURE_C): sensor.sensor_schema(
                unit_of_measurement=UNIT_KELVIN,
                accuracy_decimals=2,
                device_class=DEVICE_CLASS_ENERGY,
                state_class=STATE_CLASS_MEASUREMENT,
            ),
            cv.Optional(CONF_POWER): sensor.sensor_schema(
                unit_of_measurement=UNIT_WATT,
                accuracy_decimals=0,
                device_class=DEVICE_CLASS_ENERGY,
                state_class=STATE_CLASS_MEASUREMENT,
            ),
            cv.Optional(CONF_FLOW_RATE): sensor.sensor_schema(
                unit_of_measurement="L/min",
                accuracy_decimals=2,
                device_class=DEVICE_CLASS_ENERGY,
                state_class=STATE_CLASS_MEASUREMENT,
            ),
            cv.Optional(CONF_VOLUME): sensor.sensor_schema(
                unit_of_measurement=UNIT_CUBIC_METER,
                accuracy_decimals=0,
                device_class=DEVICE_CLASS_ENERGY,
                state_class=STATE_CLASS_MEASUREMENT,
            ),
            cv.Optional(CONF_ENERGY): sensor.sensor_schema(
                unit_of_measurement=UNIT_KILOWATT_HOURS,
                accuracy_decimals=0,
                device_class=DEVICE_CLASS_ENERGY,
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
    if CONF_FLOW_RATE in config:
        conf = config[CONF_FLOW_RATE]
        sens = await sensor.new_sensor(conf)
        cg.add(var.set_flow_rate_sensor(sens))
    if CONF_VOLUME in config:
        conf = config[CONF_VOLUME]
        sens = await sensor.new_sensor(conf)
        cg.add(var.set_volume_sensor(sens))
    if CONF_ENERGY in config:
        conf = config[CONF_ENERGY]
        sens = await sensor.new_sensor(conf)
        cg.add(var.set_energy_sensor(sens))
