#include "run_host.h"

#define mqtt_host "localhost"
#define mqtt_port 1883
#define SUB_TOPIC "LPWA/#"
//#define PUB_TOPIC "LPWA/rx" // LPWA/device_id (order)/tx or rx.

/* UART object */
UART uart_obj;

/* MQTT variables */
struct mosquitto *mosq;

/* MQTT functions */
void ConnectCallback(struct mosquitto *mosq, void *obj, int result)
{
   printf("connect callback, rc=%d\n", result);
}

void MessageCallback(struct mosquitto *mosq, void *obj, const struct mosquitto_message *message)
{
   bool match = 0;
   /*printf("got message '%.*s' for topic '%s'\n", message->payloadlen, (char*) message->payload, message->topic);*/
	printf("Receive message(%s) : %s",message->topic, message->payload);
  
   // below doesn't work
   /*mosquitto_topic_matches_sub("/LPWA/#/+", message->topic, &match);
   if (match){
      printf("got message for ADC topic\n");
   }*/

   /* When MQTT msg has been arrived from the server, 
      pass the msg to one of disposable devices using a specific "device id". 
   /* Send Uplink Packet -- to do: callback, depending on the publishing data.... */
   uart_obj.DecodeMqttTopic((char*)message->topic);
}

void PubMQTT(struct mosquitto *mosq){
   std::cout<<"Send MQTT Topic"<<std::endl;

   //std::cout<<"check UART type: "<<type<<std::endl;
   //std::cout<<"check UART address: "<<uart_obj.in_packet->device_address<<std::endl;
   //std::cout<<"check UART frequency: "<<uart_obj.in_packet->frequency<<std::endl;
   //std::cout<<"check UART payload_length: "<<uart_obj.in_packet->payload_length<<std::endl;
   //std::cout<<"check UART payload: "<<uart_obj.in_packet->payload<<std::endl;

   int payload_length = uart_obj.in_packet->payload_length;
   char msg[payload_length];

   strcpy(msg, (char*)uart_obj.in_packet->payload);
   msg[payload_length++] = '\n';

   //std::cout<<"chek msg: "<<msg<<std::endl;
   std::string add = std::to_string(uart_obj.in_packet->device_address);
   std::string temp_topic = "lpwa//rx";
   temp_topic.insert(5, add);
   const char* pub_topic = temp_topic.c_str();

   //std::cout<<"test topic: "<<pub_topic<<std::endl;

   int ret = mosquitto_publish(mosq, NULL, pub_topic, payload_length, msg, 0, false);
   if (ret) {
      printf("Cant connect to mosquitto server\n");
      exit(-1);
   }
}

void InitMqtt(){
   mosquitto_connect_callback_set(mosq, ConnectCallback);
   mosquitto_message_callback_set(mosq, MessageCallback);

   mosquitto_connect(mosq, mqtt_host, mqtt_port, 6);//Connect mosqutiio server, the last argument (keepalive) must be higher than 5. 
   mosquitto_subscribe(mosq, NULL, SUB_TOPIC, 0);//Subscribe
}

int main(int argc, char** argv){

   /* Initialize MQTT */
   int rc = 0;
   mosquitto_lib_init();
   mosq = mosquitto_new(NULL, true, NULL);
   if (!mosq) {
      printf("Cant initiallize mosquitto library\n");
      exit(-1);
   }
   else {
      InitMqtt();
   }

   /* Check UART status */
   if (uart_obj.uard_fd < 0){
      std::cout<<"failed to open UART device!!"<<std::endl;
		return -1;
	}

   /* Listen Downlink Packet */
   u8	pilot_byte; 
   while(1){
      /* Check mqtt connection */
      rc = mosquitto_loop(mosq, 0, 1);
      if(rc){
         printf("MQTT connection error!\n");
         sleep(10);
         mosquitto_reconnect(mosq);
      }

      /* Read single byte from the UART signal  */
      int length = read(uart_obj.uard_fd, &pilot_byte, 1);
      if (length == 1)
      {
         switch (pilot_byte){
            case 0xFE: // In case of "Start byte"
               uart_obj.uart_packet_length = 0;
               std::cout<<"Start flag detected"<<std::endl;
               break;
            case 0xFF: // In case of "End byte"
               if (uart_obj.uart_packet_length > 0)
               {
                  std::cout<<"End flag detected"<<std::endl;
                  std::cout<<"Length of received packet: "<<uart_obj.uart_packet_length<<std::endl;
                  uart_obj.DecodePacket();
                  PubMQTT(mosq);
               }
               break;
            default: // In the middle of receiving progress, stack the data!
               uart_obj.uart_packet_buffer[uart_obj.uart_packet_length++] = pilot_byte;
               //std::cout<<"stacked! len of packet: "<<uart_obj.uart_packet_length<<std::endl;
               break;
         }
      }
   }
   mosquitto_destroy(mosq);
   mosquitto_lib_cleanup();
}
