#pragma once

#include "esphome/core/component.h"
#include "esphome/components/uart/uart.h"
#include "esphome/components/sensor/sensor.h"


#include "mbus-protocol.h"


namespace esphome {
namespace engelmannMBus {

class EngelmannMBus : public PollingComponent, public uart::UARTDevice {
 public:
  // constructor
  EngelmannMBus() : PollingComponent(15000) {}

  void set_flow_temperature_sensor(sensor::Sensor *flow_temperature_sensor) { flow_temperature_sensor_ = flow_temperature_sensor; }

  float get_setup_priority() const override 
  { return esphome::setup_priority::LATE; }

  void setup() override;
  void loop() override;
  void update() override;

  int mbus_serial_recv_frame(mbus_frame *frame);
protected:
  sensor::Sensor *flow_temperature_sensor_{nullptr};
private:
  bool req_sent = false;
};
}
}