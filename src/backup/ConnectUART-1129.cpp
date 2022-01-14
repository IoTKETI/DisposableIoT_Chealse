#include "ConnectUART.h"

#define mqtt_host "localhost"
#define mqtt_port 1883
#define SUB_TOPIC "LPWA/#"
#define PUB_TOPIC "LPWA/test"

const bool test=1; // 0: write test, 1: read test

/* MQTT variables */
struct mosquitto *mosq;

/* UART variables */
int uart_packet_length = 0;
char uart_packet_buffer[2048];

int interface_packet_length = 0;
char interface_packet[2048];

/* MQTT functions */
void connect_callback(struct mosquitto *mosq, void *obj, int result)
{
   printf("connect callback, rc=%d\n", result);
}

void message_callback(struct mosquitto *mosq, void *obj, const struct mosquitto_message *message)
{
   bool match = 0;
   /*printf("got message '%.*s' for topic '%s'\n", message->payloadlen, (char*) message->payload, message->topic);*/
	printf("Receive message(%s) : %s",message->topic, message->payload);

   // below doesn't work
   mosquitto_topic_matches_sub("/devices/wb-adc/controls/+", message->topic, &match);
   if (match) {
      printf("got message for ADC topic\n");
   }
}

/* UART functions */
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

void decode_packet(MAC_UPLINK_PACKET_TYPE *pPacket, struct mosquitto *mosq){
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

   char test[payload_length];
   strcpy(test, (char*)payload);
   test[payload_length++] = '\n';

   int ret = mosquitto_publish(mosq, NULL, PUB_TOPIC, strlen(test), test, 0, false);
   if (ret) {
      printf("Cant connect to mosquitto server\n");
      exit(-1);
   }

}

void init_mqtt(){
   mosquitto_connect_callback_set(mosq, connect_callback);
   mosquitto_message_callback_set(mosq, message_callback);

   mosquitto_connect(mosq, mqtt_host, mqtt_port, 6);//mosqutiio 서버와 연결, 마지막 인수 (keepalive)가 반드시 5이상 이어야만 연결됨..
   mosquitto_subscribe(mosq, NULL, SUB_TOPIC, 0);//subscribe
}

int main(int argc, char** argv){
   int rc = 0;
   mosquitto_lib_init();
   mosq = mosquitto_new(NULL, true, NULL);

   if (!mosq) {
      printf("Cant initiallize mosquitto library\n");
      exit(-1);
   }
   else {
      init_mqtt();
   }

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
         /* mqtt loop check */
         rc = mosquitto_loop(mosq, 0, 1);
         if(rc){
            printf("MQTT connection error!\n");
            sleep(10);
            mosquitto_reconnect(mosq);
         }

         /* read each byte from the UART signal  */
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
                     decode_packet(pPacket, mosq);

                     // pub을 function안에 넣으면 동작 안함... pub은 되는데, sub이 안됨... 왜 그렇지? 다음주 check!!
                     char text[20] = "Nice to meet u!\n";

                  }
                  break;
               
               default:
                  uart_packet_buffer[uart_packet_length++] = pilot_byte;
                  std::cout<<"stacked! len of packet: "<<uart_packet_length<<std::endl;
                  break;
            }
         }
      }
      mosquitto_destroy(mosq);
   }

   mosquitto_lib_cleanup();
}
