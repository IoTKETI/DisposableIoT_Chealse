#ifndef __run_host_h
#define __run_host_h

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <termios.h>
#include <fcntl.h>
#include <iostream>

/*add custom library */
#include "uart_handler.h"

/* add mqtt library */
#include <mosquitto.h>

void ConnectCallback(struct mosquitto *mosq, void *obj, int result);
void MessageCallback(struct mosquitto *mosq, void *obj, const struct mosquitto_message *message);
void Initqtt();

void PubMQTT(struct mosquitto *mosq);

#endif