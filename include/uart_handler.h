#ifndef __uart_handler_h
#define __uart_handler_h

#include "interface.h"
#include "base64.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <sys/types.h>
#include <termios.h>
#include <fcntl.h>
#include <iostream>
#include <map>

class UART{
private:
    std::map<u8, u32> address_map;
    u8 device_order;
    char* server_message;

public:
    struct termios oldtio;
    struct termios newtio;

    /* UART variables */
    int uard_fd;
    int uart_packet_length;
    char uart_packet_buffer[2048];

    int interface_packet_length;
    char interface_packet[2048];

    MAC_UPLINK_PACKET_TYPE *in_packet;

    MAC_DOWNLINK_PACKET_TYPE down_packet;
    char encoded_msg[2048];

public:
    void OpenUART();
    int SendPacket(u32 msg_len, char* mqtt_msg);
    int GetMessage(u8 *message);
    void DecodePacket();

    void DecodeMqttTopic(char* mqtt_topic);
    
    UART();
    ~UART();
};

#endif