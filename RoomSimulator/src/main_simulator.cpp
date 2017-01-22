#include "Room.h"

#include <stdio.h>
#include <iostream>
#include <mosquitto.h>

#include <rapidjson/rapidjson.h>
#include <rapidjson/document.h>
#include <rapidjson/stringbuffer.h>
#include <rapidjson/writer.h>

#include <chrono>
#include <thread>

// this is a room with a valve and a temperature sensor. mosquitto basic example was taken as the basis

struct mosquitto* g_mosq = NULL;
Room g_simpleRoom;

void sendTemperatureSensorMessage() {
    using namespace rapidjson;
    // sensor message template
    const char* jsonStr = "{            \
        \"sensorID\": \"sensor-1\",     \
        \"type\": \"temperature\",      \
        \"value\": 25.3                 \
        }";
    Document d;
    d.Parse(jsonStr);
    Value& t = d["value"];
    t.SetFloat(g_simpleRoom.getTemperature());

    StringBuffer buffer;
    Writer<StringBuffer> writer(buffer);

    d.Accept(writer);
    const char* output = buffer.GetString();

    mosquitto_publish(g_mosq, nullptr, "/readings/temperature", buffer.GetLength(), buffer.GetString(), 2, false);
}

void receiveValveMessage(char* jsonStr) {
    using namespace rapidjson;
    Document d;
    d.Parse(jsonStr);
    if (d.HasParseError() || !d.IsObject() || !d.HasMember("level")) {
        std::cout << "Invalid Json" << std::endl;
        return;
    }
    Value& l = d["level"];
    g_simpleRoom.setValve(l.GetInt());
}

void my_message_callback(struct mosquitto *mosq, void *userdata, const struct mosquitto_message *message)
{
    if (message->payloadlen) {
        receiveValveMessage(static_cast<char*>(message->payload));
    }
    else {
        printf("%s (null message)\n", message->topic);
    }
    fflush(stdout);
}

void my_connect_callback(struct mosquitto *mosq, void *userdata, int result)
{
    if (!result) {
        // room simulator connects to valve commands' topic
        mosquitto_subscribe(mosq, NULL, "/actuators/room-1", 2);
    }
    else {
        fprintf(stderr, "Connect failed\n");
    }
}

void my_subscribe_callback(struct mosquitto *mosq, void *userdata, int mid, int qos_count, const int *granted_qos)
{
    int i;

    printf("Subscribed (mid: %d): %d", mid, granted_qos[0]);
    for (i = 1; i<qos_count; i++) {
        printf(", %d", granted_qos[i]);
    }
    printf("\n");
}

void my_log_callback(struct mosquitto *mosq, void *userdata, int level, const char *str)
{
    /* Pring all log messages regardless of level. */
    printf("%s\n", str);
}

int main(int argc, char *argv[])
{
    char *host = "localhost";
    int port = 1883;
    int keepalive = 60;
    bool clean_session = true;

    mosquitto_lib_init();
    g_mosq = mosquitto_new(NULL, clean_session, NULL);
    if (!g_mosq) {
        fprintf(stderr, "Error: Out of memory.\n");
        return 1;
    }
    mosquitto_log_callback_set(g_mosq, my_log_callback);
    mosquitto_connect_callback_set(g_mosq, my_connect_callback);
    mosquitto_message_callback_set(g_mosq, my_message_callback);
    mosquitto_subscribe_callback_set(g_mosq, my_subscribe_callback);

    if (mosquitto_connect(g_mosq, host, port, keepalive)) {
        fprintf(stderr, "Unable to connect.\n");
        return 1;
    }
    int step = 0;

    // do the loop manually to also run the simulation
    while (1) {
        // simulate room with 0.1 s resolution
        ++step;
        std::this_thread::sleep_for(std::chrono::milliseconds(1000));
        mosquitto_loop(g_mosq, -1, 1);
        g_simpleRoom.tick();
        // send temperature update every second
        //if (step % 10) {
        sendTemperatureSensorMessage();
        //}
        std::cout << "\nTemperature: " << g_simpleRoom.getTemperature() << std::endl;
    }

    mosquitto_destroy(g_mosq);
    mosquitto_lib_cleanup();
    return 0;
}
