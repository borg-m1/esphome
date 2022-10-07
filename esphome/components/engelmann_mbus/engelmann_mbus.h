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

  void set_temperature_sensor(sensor::Sensor *temperature_sensor) { temperature_sensor_ = temperature_sensor; }

  float get_setup_priority() const override 
  { return esphome::setup_priority::LATE; }

  void setup() override;
  void loop() override;
  void update() override;

  int mbus_serial_recv_frame(mbus_frame *frame);
protected:
  sensor::Sensor *temperature_sensor_{nullptr};
private:
  bool req_sent = false;
};
}
}