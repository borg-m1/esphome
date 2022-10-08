#include "engelmann_mbus.h"
#include "mbus-protocol.h"
#include "arch/cc.h"
#include <string.h>

#include <tinyxml2.h>
#define PACKET_BUFF_SIZE 2048


namespace esphome {
namespace engelmannMBus {

static const char *const TAG = "engelmann_mbus";


static const uint8_t MBUS_REQ_UD2[] = {0x10, 0x5b, 0x00, 0x5b, 0x16};


void EngelmannMBus::setup() {
    mbus_parse_set_debug(0);
}

void EngelmannMBus::loop() {
    mbus_frame frame;
    mbus_frame_data reply_data;
    char *xml_result;
    tinyxml2::XMLDocument doc;

    /*
    uint8_t test [2];

    if (this->req_sent && this->available()) {
        int read;
        read = this->read_array(test, 2);
        if(read == 2)
            ESP_LOGD("hallo", "value1: %.2X, value2: %.2X", test[0], test[1]);
        else
            ESP_LOGD("hallo", "read was: %d", read);
    }
    */
    
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
			ESP_LOGD("hallo", "%s", name);
            if(std::string(name) == "DataRecord")
                std::string abcdef(ele->FirstChildElement("Unit")->GetText());
                std::string abcdef(ele->FirstChildElement("Unit")->GetText());
                ESP_LOGD("hallo", "Unit: %s", abcdef.c_str());
                std::size_t found = abcdef.find("Flow temperature");
                if (found!=std::string::npos) {
                    std::string value(ele->FirstChildElement("Value")->GetText());
                    ESP_LOGD("hallo", "Temperature: %s", value.c_str());
                }


		}

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