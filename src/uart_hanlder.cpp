#include "uart_handler.h"

UART::UART(){
    this->oldtio = { 0 };
    this->newtio = { 0 };
    this->uart_packet_length = 0;
    this->interface_packet_length = 0;
    memset(this->uart_packet_buffer, 0x00, sizeof(this->uart_packet_buffer));
    memset(this->interface_packet, 0x00, sizeof(this->interface_packet));
    memset(this->encoded_msg, 0x00, sizeof(this->encoded_msg));

    this->OpenUART();

    /* Store the address of our disposable devices */
    this->address_map.insert(std::pair<u8, u32>('0', 11272304));
}

UART::~UART(){
    //delete this->uart_packet_buffer;
    //delete this->interface_packet;
}

void UART::OpenUART(){
    this->uard_fd = open("/dev/ttyUSB0", O_RDWR | O_NOCTTY);
    tcgetattr(this->uard_fd, &this->oldtio);

    this->newtio.c_cflag = B115200 | CS8 | CLOCAL | CREAD;  // speed 115200, data bit 8bit, internal port
                                                      // write is default (allow to read)
    this->newtio.c_cflag &= ~CSTOPB; // stop bit = 1 bit.
    this->newtio.c_iflag = 0; // IGNPAR | ICRNL // No parity bit
    this->newtio.c_oflag = 0;
    this->newtio.c_lflag = 0; // ICANON
    this->newtio.c_cc[VTIME] = 0;
    this->newtio.c_cc[VMIN] = 1;
    tcflush(this->uard_fd, TCIOFLUSH);
    tcsetattr(this->uard_fd, TCSANOW, &this->newtio); // communication environment for the port

    //Set to non-blocking mode, this will be used when reading the serial port
    fcntl(this->uard_fd, F_SETFL, O_NONBLOCK);
}

int UART::SendPacket(u32 msg_len, char* mqtt_msg){
    //std::cout<<"check msg len: "<<msg_len<<std::endl;
    //std::cout<<"check msg: "<<mqtt_msg<<std::endl;
    this->server_message = mqtt_msg;

    /* Set Packet */
    this->down_packet.type = ED_DOWNLINK_PACKET;
    this->down_packet.device_address = this->address_map[this->device_order]; // it should be changed per device's address.

    int message_length = this->GetMessage(this->down_packet.payload);
    this->down_packet.payload_length = message_length;
    //std::cout<<"len_message: "<<message_length<<std::endl;

    this->encoded_msg[0] = 0xFE; // add start flag
    /* base64 Encoding */
    int length = bin_to_b64((unsigned char *)&this->down_packet, sizeof(MAC_DOWNLINK_PACKET_TYPE), (char *)&this->encoded_msg[1], 2048);
    this->encoded_msg[length+1] = 0xFF; // add end flag

    int ret = write(this->uard_fd, this->encoded_msg, length+2);
    if (ret != length+2)
    {
        std::cout<<"error happened"<<std::endl;
    }

    return ret;
}

/* Temporary function. It could be unified with the upper function following a direction from Dr. Lee. */
void UART::DecodeMqttTopic(char* mqtt_topic){
    //std::cout<<"check data: "<<mqtt_topic<<std::endl;
    this->device_order = mqtt_topic[5];
}

int UART::GetMessage(u8 *message){
    strcpy((char *) message, this->server_message);
    //std::cout<<"chek len of char: "<<strlen((char*)message)<<std::endl;
    //std::cout<<"check copied msg: "<<message<<std::endl;

    /*for (int i=0; i<server_message.size(); ++i){
       message[i] = server_message[i];
    }*/

   return strlen((char*)message)-1; // Return len without 'NULL' value.
}

void UART::DecodePacket(){
    this->interface_packet_length = b64_to_bin((const char *)this->uart_packet_buffer, this->uart_packet_length
                                             , (unsigned char *)&this->interface_packet, 2048);
    in_packet = (MAC_UPLINK_PACKET_TYPE *)this->interface_packet;

    //std::cout<<"test address: "<<in_packet->device_address<<std::endl;
}