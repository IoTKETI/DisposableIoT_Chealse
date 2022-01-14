#include "ConnectUART.h"

bool test=1; // 0: write test, 1: read test

int uart_packet_length = 0;
char uart_packet_buffer[2048];

int interface_packet_length = 0;
char interface_packet[2048];

int openUART(){
    int fd = open("/dev/ttyUSB0", O_RDWR | O_NOCTTY);
    struct termios oldtio = { 0 };
    struct termios newtio = { 0 };
    tcgetattr(fd, &oldtio);

    newtio.c_cflag = B115200 | CS8 | CLOCAL | CREAD;  // speed 115200, data bit 8bit, internal port
                                                      // write is default (allow to read)
    newtio.c_cflag &= ~CSTOPB; // stop bit = 1 bit.
    newtio.c_iflag = 0; // IGNPAR | ICRNL // No parity bit
    newtio.c_oflag = 0;
    newtio.c_lflag = 0; // ICANON
    newtio.c_cc[VTIME] = 0;
    newtio.c_cc[VMIN] = 1;
    tcflush(fd, TCIOFLUSH);
    tcsetattr(fd, TCSANOW, &newtio); // communication environment for the port

    //Set to non-blocking mode, this will be used when reading the serial port
    fcntl(fd, F_SETFL, O_NONBLOCK);
    return fd;
}

int get_message(u8 *message){
   std::string server_message = "hello";
   //strcpy((char *) message, server_message.c_str());
   for (int i=0; i<server_message.size(); ++i){
      message[i] = server_message[i];
   }

   return server_message.size();
}

int send_packet(int uart)
{
	MAC_DOWNLINK_PACKET_TYPE packet;
   char encoded_msg[2048];

   /* Set Packet */
   packet.type		= ED_DOWNLINK_PACKET;
   packet.device_address = (u32)11272304;

   int message_length = get_message(packet.payload);
   packet.payload_length = message_length;
   std::cout<<"len_message: "<<message_length<<std::endl;

   encoded_msg[0] = 0xFE; // add start flag
   /* base64 Encoding */
   int length = bin_to_b64((unsigned char *)&packet, sizeof(MAC_DOWNLINK_PACKET_TYPE), (char *)&encoded_msg[1], 2048);
   encoded_msg[length+1] = 0xFF; // add end flag

   int ret = write(uart, encoded_msg, length+2);
   if (ret != length+2)
   {
      std::cout<<"error happened"<<std::endl;
   }

   return ret;
}

void decode_packet(MAC_UPLINK_PACKET_TYPE *pPacket){
   std::cout<<"Start decoding"<<std::endl;

   u32 type = pPacket->type;
   u32 frequency = pPacket->frequency;
   double rssi = pPacket->rssi;
   u32 address = pPacket->device_address;
   u16 payload_length = pPacket->payload_length;
   u8 *payload = pPacket->payload;

   //std::cout<<"check type: "<<type<<std::endl;
   std::cout<<"check address: "<<address<<std::endl;
   std::cout<<"check frequency: "<<frequency<<std::endl;
   std::cout<<"check payload_length: "<<payload_length<<std::endl;
   std::cout<<"check payload: "<<payload<<std::endl;
}

int main(int argc, char** argv){
   struct mosquitto *mosq;
   
   int uart = openUART(); // serial port descriptor 
   if (uart < 0){
      std::cout<<"failed to open UART device!!"<<std::endl;
		return -1;
	}
   memset(uart_packet_buffer, 0x00, sizeof(uart_packet_buffer));

   u8	pilot_byte; 

   if (test==false){
      /* Send Uplink Packet -- to do: callback */
      for(int count=0; count<2; ++count){
         int ret = send_packet(uart);
         std::cout<<"check ret: "<<ret<<std::endl;
         sleep(1);
      }
   }
   else{
      /* Listen Downlink Packet */
      while(1){
      //for(int count=0; count<100; ++count) {
         //std::cout<<"Listening..."<<std::endl;
         int length = read(uart, &pilot_byte, 1);
         if (length == 1)
         {
            switch (pilot_byte){
               case 0xFE:
                  uart_packet_length = 0;
                  std::cout<<"Start stacking"<<std::endl;
                  break;
               case 0xFF:
                  if (uart_packet_length > 0)
                  {
                     std::cout<<"End stacking"<<std::endl;

                     interface_packet_length = b64_to_bin((const char *)uart_packet_buffer, uart_packet_length
                                             , (unsigned char *)&interface_packet, 2048);
                     //mac_task_send_primitive_from_isr(ENTITY_PDCP, ENTITY_PDCP, PDCP_TRANSMIT_PACKET_REQUEST, interface_packet_length, interface_packet);

                     MAC_UPLINK_PACKET_TYPE * pPacket = (MAC_UPLINK_PACKET_TYPE *)interface_packet;
                     decode_packet(pPacket);
                  }
                  break;

               default:
                  uart_packet_buffer[uart_packet_length++] = pilot_byte;
                  std::cout<<"stacked! len of packet: "<<uart_packet_length<<std::endl;
                  break;
            }
         }
      }

   }
}
