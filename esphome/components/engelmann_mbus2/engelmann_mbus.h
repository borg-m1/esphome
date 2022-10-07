#pragma once

#include "esphome/core/component.h"
#include "esphome/components/light/addressable_light_effect.h"
#include "esphome/components/uart/uart.h"

#include "libmbus/mbus-protocol.h"


namespace esphome {
namespace engelmannMBus {

class EngelmannMBus : public PollingComponent, public uart::UARTDevice {
 public:
  // constructor
  EngelmannMBus() : PollingComponent(15000) {}

  float get_setup_priority() const override 
  { return esphome::setup_priority::LATE; }

  void setup() override;
  void loop() override;
  void update() override;

  int mbus_serial_recv_frame(mbus_frame *frame);
private:
  bool req_sent = false;
};
}
}