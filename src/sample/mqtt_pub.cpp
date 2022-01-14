#include <signal.h>
#include <errno.h>
#include <fcntl.h>
#include <stdlib.h>
#include <string.h>
#include <mosquitto.h>
#include <stdio.h>

#define MQTT_HOSTNAME "localhost"
#define MQTT_PORT 1883
#define MQTT_USERNAME "admin"
#define MQTT_PASSWORD "admin"
#define MQTT_TOPIC "LPWA/test"

//#define LPWA_TX "LPWA/id/tx"
//#define LPWA_RX "LPWA/id/rx" 
// 추후 wildcard 참조 LPWA/id/# --> 하위 토픽 전부 subcribe

static int run = 1;

void handle_signal(int s)
{
   run = 0;
}

void connect_callback(struct mosquitto *mosq, void *obj, int result)
{
   printf("connect callback, rc=%d\n", result);
}

int main(int argc, char **argv) {
   struct mosquitto *mosq;
   int ret = 0;

   // 초기화
   mosquitto_lib_init();

   // 모스키토 런타임 객체와 클라이언트 랜덤 ID 생성
   mosq = mosquitto_new(NULL, true, NULL);
   if (!mosq) {
      printf("Cant initiallize mosquitto library\n");
      exit(-1);
   }

   //mosquitto_username_pw_set(mosq, MQTT_USERNAME, MQTT_PASSWORD);

   // MQTT 서버 연결 설립, keep-alive 메시지 사용 안함
   //mosquitto_connect_callback_set(mosq, connect_callback);
   ret = mosquitto_connect(mosq, MQTT_HOSTNAME, MQTT_PORT, 6);


   if (ret) {
      printf("Cant connect to mosquitto server\n");
      exit(-1);
   }

   char text[20] = "Nice to meet u!\n";

   ret = mosquitto_publish(mosq, NULL, MQTT_TOPIC, strlen(text), text, 0, false);
   if (ret) {
      printf("Cant connect to mosquitto server\n");
      exit(-1);
   }

   // 네트워크 동작이 끝나기 전에 모스키토 동작을 막기위해 잠깐의 딜레이가 필요함
   //sleep(1);

   mosquitto_disconnect(mosq);
   mosquitto_destroy(mosq);
   mosquitto_lib_cleanup();

   return 0;
}