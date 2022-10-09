#include "engelmann_mbus.h"
#include "mbus-protocol.h"
#include "arch/cc.h"
#include <string.h>
#include <map>

#include <tinyxml2.h>
#define PACKET_BUFF_SIZE 2048


namespace esphome {
namespace engelmannMBus {

static const char *const TAG = "engelmann_mbus";


static const uint8_t MBUS_REQ_UD2[] = {0x10, 0x5b, 0x00, 0x5b, 0x16};

std::map<std::string, float> values;

void EngelmannMBus::setup() {
    mbus_parse_set_debug(0);
}

void EngelmannMBus::loop() {
    mbus_frame frame;
    mbus_frame_data reply_data;
    char *xml_result;
    tinyxml2::XMLDocument doc;

    
    if (this->req_sent && this->available()) {
        ESP_LOGD("hallo", "UART available!");
        if(mbus_serial_recv_frame(&frame) != MBUS_RECV_RESULT_OK) {
            return;
        }
        ESP_LOGD("hallo", "MBUS frame received");
        this->req_sent = false;
        if(mbus_frame_data_parse(&frame, &reply_data) == 0) {
            ESP_LOGD("hallo", "MBUS parse successful!");
            
        }
        else {
            ESP_LOGD("hallo", "MBUS parse failed!");
            return;
        }
            
        this->flush();
        if ((xml_result = mbus_frame_data_xml(&reply_data)) == NULL)
        {
            ESP_LOGD("hallo", "Failed to generate XML representation of MBUS frame: %s", mbus_error_str());

            return;
        }
        //ESP_LOGD("hallo", "%s", xml_result);

        if(doc.Parse(xml_result) == tinyxml2::XMLError::XML_SUCCESS)
            ESP_LOGD("hallo", "doc parse success!");
        const char* title = doc.FirstChildElement( "MBusData" )->FirstChildElement( "SlaveInformation" )->FirstChildElement( "ProductName" )->GetText();
        
        for( tinyxml2::XMLElement* ele = doc.FirstChildElement( "MBusData" )->FirstChildElement();
			 ele;
			 ele = ele->NextSiblingElement() )
		{
            const char* name = ele->Name();
			//ESP_LOGD("hallo", "%s", name);
            if(std::string(name) == "DataRecord" && 
               std::string(ele->FirstChildElement("Function")->GetText()) == "Instantaneous value" &&
               std::string(ele->FirstChildElement("StorageNumber")->GetText()) == "0" &&
               !ele->FirstChildElement("Tariff") &&
               std::string(ele->FirstChildElement("Unit")->GetText()) != "Time Point (time & date)" &&
               std::string(ele->FirstChildElement("Unit")->GetText()) != "Fabrication number" &&
               std::string(ele->FirstChildElement("Unit")->GetText()) != "Model / Version") {
                std::string unit_str(ele->FirstChildElement("Unit")->GetText());
                //ESP_LOGD("hallo", "Unit: %s", unit_str.c_str());

                const char* value = ele->FirstChildElement("Value")->GetText();
                //ESP_LOGD("hallo", "Value: %s", value);
                
                values.insert(std::make_pair(unit_str, atof(value)));
            }

		}

        for(std::map<std::string, float >::const_iterator it = values.begin();
                it != values.end(); ++it) {
                //ESP_LOGD("hallo", "Key: %s, value: %f", it->first.c_str(), it->second);
                if (it->first == "Flow temperature (deg C)") {
                    if (flow_temperature_sensor_ != nullptr) {
                        flow_temperature_sensor_->publish_state(it->second);
                    }
                }
                if (it->first == "Return temperature (deg C)") {
                    if (return_temperature_sensor_ != nullptr) {
                        return_temperature_sensor_->publish_state(it->second);
                    }
                }
                if (it->first == "Temperature Difference (1e-2  deg C)") {
                    if (difference_temperature_sensor_ != nullptr) {
                        difference_temperature_sensor_->publish_state(it->second * 1e-2);
                    }
                }
                if (it->first == "Power (W)") {
                    if (power_sensor_ != nullptr) {
                        power_sensor_->publish_state(it->second);
                    }
                }
                if (it->first == "Volume flow (m m^3/h)") {
                    if (flow_rate_sensor_ != nullptr) {
                        flow_rate_sensor_->publish_state(it->second * 1e-3 * 16.6667);
                    }
                }
                if (it->first == "Volume (m m^3)") {
                    if (volume_sensor_ != nullptr) {
                        volume_sensor_->publish_state(it->second * 1e-3);
                    }
                }
                if (it->first == "Energy (kWh)") {
                    if (energy_sensor_ != nullptr) {
                        energy_sensor_->publish_state(it->second);
                    }
                }
                
            }
            values.clear();

        //if(title != 0)
        //    ESP_LOGD("hallo", "%s", title);
        

        free(xml_result);

            // manual free
        if (reply_data.data_var.record)
        {
            mbus_data_record_free(reply_data.data_var.record); // free's up the whole list
        }
    }
    
}

// called every n ms (PollingComponent)
void EngelmannMBus::update() {
    this->write_array(MBUS_REQ_UD2, sizeof(MBUS_REQ_UD2));
    //this->flush();

    this->req_sent = true;
}

int EngelmannMBus::mbus_serial_recv_frame(mbus_frame *frame)
{
    uint8_t buff[PACKET_BUFF_SIZE];
    int len, remaining, nread, timeouts;


    if (frame == NULL)
    {
        //printf_P(PSTR("%s: Invalid parameter.\n"), __PRETTY_FUNCTION__);
        return -1;
    }

    memset(buff, 0, sizeof(buff));

    //
    // read data until a packet is received
    //
    remaining = 1; // start by reading 1 byte
    len = 0;
    timeouts = 0;
    do {
        //printf_P(PSTR("%s: Attempt to read %d bytes [len = %d]\n"), __PRETTY_FUNCTION__, remaining, len);
    	while(this->available()>0 && len<PACKET_BUFF_SIZE)
    	{
    		buff[len] = this->read();
    		len++;
    	}


    } while ((remaining = mbus_parse(frame, buff, len)) > 0);

    if (len == 0)
    {
        // No data received
        return -1;
    }
    
      
    if (remaining != 0)
    {
        // Would be OK when e.g. scanning the bus, otherwise it is a failure.
        //printf_P(PSTR("%s: M-Bus layer failed to receive complete data.\n"), __PRETTY_FUNCTION__);
        return -2;
    }

    if (len == -1)
    {
        //printf_P(PSTR("%s: M-Bus layer failed to parse data.\n"), __PRETTY_FUNCTION__);
        return -1;
    }

    ESP_LOGD("hallo", "parsed %d bytes", len);

    return 0;
}
}
}