#include "engelmann_mbus.h"
#include "libmbus/mbus-protocol.h"
#include "arch/cc.h"

#define PACKET_BUFF_SIZE 2048


namespace esphome {
namespace engelmannMBus {

static const uint8_t MBUS_REQ_UD2[] = {0x10, 0x5b, 0x00, 0x5b, 0x16};


void EngelmannMBus::setup() {

}

void EngelmannMBus::loop() {
    mbus_frame frame;
    mbus_frame_data frame_data;
    
    if (this->req_sent && this->available()) {
        if(mbus_serial_recv_frame(&frame) != MBUS_RECV_RESULT_OK) {
            return;
        }
        this->req_sent = false;
        if(mbus_frame_data_parse(&frame, &frame_data) == 0) {
            ESP_LOGD("hallo", "MBUS parse successful!");
        }
    }
}

// called every n ms (PollingComponent)
void EngelmannMBus::update() {
    this->write_array(MBUS_REQ_UD2, sizeof(MBUS_REQ_UD2));
    this->flush();

    this->req_sent = true;
}

int EngelmannMBus::mbus_serial_recv_frame(mbus_frame *frame)
{
    uint8_t buff[PACKET_BUFF_SIZE];
    int remaining, timeouts;
    ssize_t len, nread;

    if (frame == NULL)
    {
        fprintf(stderr, "%s: Invalid parameter.\n", __PRETTY_FUNCTION__);
        return MBUS_RECV_RESULT_ERROR;
    }

    memset((void *)buff, 0, sizeof(buff));

    //
    // read data until a packet is received
    //
    remaining = 1; // start by reading 1 byte
    len = 0;
    timeouts = 0;

    do {
        if (len + remaining > PACKET_BUFF_SIZE)
        {
            // avoid out of bounds access
            return MBUS_RECV_RESULT_ERROR;
        }

        //printf("%s: Attempt to read %d bytes [len = %d]\n", __PRETTY_FUNCTION__, remaining, len);

        if ((nread = this->read_array(&buff[len], remaining)) == -1)
        {
       //     fprintf(stderr, "%s: aborting recv frame (remaining = %d, len = %d, nread = %d)\n",
         //          __PRETTY_FUNCTION__, remaining, len, nread);
            return MBUS_RECV_RESULT_ERROR;
        }

//   printf("%s: Got %d byte [remaining %d, len %d]\n", __PRETTY_FUNCTION__, nread, remaining, len);

        if (nread == 0)
        {
            timeouts++;

            if (timeouts >= 3)
            {
                // abort to avoid endless loop
                fprintf(stderr, "%s: Timeout\n", __PRETTY_FUNCTION__);
                break;
            }
        }

        if (len > (SSIZE_MAX-nread))
        {
            // avoid overflow
            return MBUS_RECV_RESULT_ERROR;
        }

        len += nread;

    } while ((remaining = mbus_parse(frame, buff, len)) > 0);

    if (len == 0)
    {
        // No data received
        return MBUS_RECV_RESULT_TIMEOUT;
    }

    if (remaining != 0)
    {
        // Would be OK when e.g. scanning the bus, otherwise it is a failure.
        // printf("%s: M-Bus layer failed to receive complete data.\n", __PRETTY_FUNCTION__);
        return MBUS_RECV_RESULT_INVALID;
    }

    if (len == -1)
    {
        fprintf(stderr, "%s: M-Bus layer failed to parse data.\n", __PRETTY_FUNCTION__);
        return MBUS_RECV_RESULT_ERROR;
    }

    return MBUS_RECV_RESULT_OK;
}

}
}