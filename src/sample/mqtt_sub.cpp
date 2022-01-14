// https://wnsgml972.github.io/mqtt/2018/02/13/mqtt_ubuntu-my_client/
// $ gcc -o my_sub my_sub.c -lmosquitto


#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <unistd.h>
#include <mosquitto.h>

#define mqtt_host "localhost"
#define mqtt_port 1883
#define MQTT_TOPIC "myTopic"

static int run = 1;

void handle_signal(int s)
{
   run = 0;
}

void connect_callback(struct mosquitto *mosq, void *obj, int result)
{
   printf("connect callback, rc=%d\n", result);
}

void message_callback(struct mosquitto *mosq, void *obj, const struct mosquitto_message *message)
{
   bool match = 0;
   /*printf("got message '%.*s' for topic '%s'\n", message->payloadlen, (char*) message->payload, message->topic);*/
	printf("receive message(%s) : %s",message->topic, message->payload);

   //뭐하는 코드인지 모름 작동 안함
   //mosquitto_topic_matches_sub("/devices/wb-adc/controls/+", message->topic, &match);
   if (match) {
      printf("got message for ADC topic\n");
   }

}

int main(int argc, char *argv[])
{
   uint8_t reconnect = true;
   //char clientid[24];//id를 사용하는 경우
   struct mosquitto *mosq;
   int rc = 0;

   signal(SIGINT, handle_signal);
   signal(SIGTERM, handle_signal);

   mosquitto_lib_init();

   //메모리 초기화
   //memset(clientid, 0, 24);//맨 앞부터 0을 24개 삽입 (초기화)
   //snprintf(clientid, 23, "mysql_log_%d", getpid());//23길이의 clientid에 pid를 가진 문자열 삽입
   // mosq = mosquitto_new(clientid, true, 0);//mosquitto 구조체 생성 <-
   mosq = mosquitto_new(NULL, true, NULL);//mosquitto 구조체 생성
   if (!mosq) {
      printf("Cant initiallize mosquitto library\n");
      exit(-1);
   }
   else {
      mosquitto_connect_callback_set(mosq, connect_callback);
      mosquitto_message_callback_set(mosq, message_callback);

      rc = mosquitto_connect(mosq, mqtt_host, mqtt_port, 6);//mosqutiio 서버와 연결, 마지막 인수 (keepalive)가 반드시 5이상 이어야만 연결됨..
      printf("test rc=%d\n", rc);

      mosquitto_subscribe(mosq, NULL, MQTT_TOPIC, 0);//subscribe

      while(run){
         rc = mosquitto_loop(mosq, -1, 1);
         if(run && rc){
            printf("connection error!\n");
            sleep(10);
            mosquitto_reconnect(mosq);
         }
      }
      mosquitto_destroy(mosq);
   }

   mosquitto_lib_cleanup();

   return rc;
}