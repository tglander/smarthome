#include "TemperatureControlStrategy.h"

#include <stdio.h>
#include <iostream>
#include <mosquitto.h>

#include <rapidjson/rapidjson.h>
#include <rapidjson/document.h>
#include <rapidjson/stringbuffer.h>
#include <rapidjson/writer.h>

// this is a temperature controller. mosquitto basic example was taken as the basis

// hard coded strategy with target temperature
BinaryHeatControl g_strategy(22.f);

struct mosquitto* g_mosq = nullptr;

char* const g_actuatorTopic = "/actuators/room-1";
char* const g_sensorTopic = "/readings/temperature";

rapidjson::StringBuffer createValveMessage(int valveLevel) {
    assert(valveLevel <= 100 && valveLevel >= 0);
    using namespace rapidjson;
    const char* jsonStr = "{\"level\": 14}";
    Document d;
    d.Parse(jsonStr);
    Value& l = d["level"];
    l.SetInt(valveLevel);

    StringBuffer buffer;
    Writer<StringBuffer> writer(buffer);

    d.Accept(writer);
    return buffer;
}

void sendValveMessage(int valveLevel) {
    assert(valveLevel <= 100 && valveLevel >= 0);
    using namespace rapidjson;
    StringBuffer buffer = createValveMessage(valveLevel);
    const char* output = buffer.GetString();
    mosquitto_publish(g_mosq, nullptr, g_actuatorTopic, buffer.GetLength(), buffer.GetString(), 2, false);
}

void receiveSensorMessageAndAct(char* jsonStr) {
    using namespace rapidjson;
    Document d;
    d.Parse(jsonStr);
    if (d.HasParseError() || !d.IsObject() || !d.HasMember("value")) {
        std::cout << "Invalid Json" << std::endl;
        return;
    }
    Value& v = d["value"];
    // maybe we need to deal with multiple sensors later
    Value& t = d["type"];
    Value& sid = d["sensorID"];

    sendValveMessage(g_strategy.computeLevel(v.GetFloat()));
}

void my_message_callback(struct mosquitto *mosq, void *userdata, const struct mosquitto_message *message)
{
    if(message->payloadlen){
        receiveSensorMessageAndAct(static_cast<char*>(message->payload));
    }else{
        printf("%s (null message)\n", message->topic);
    }
    fflush(stdout);
}

void my_connect_callback(struct mosquitto *mosq, void *userdata, int result)
{
    if(!result){
        mosquitto_subscribe(mosq, NULL, g_sensorTopic, 2);

        // shut down heating before we go to be safe
        rapidjson::StringBuffer lastWill = createValveMessage(0);
        mosquitto_will_set(mosq,
            g_actuatorTopic,
            lastWill.GetLength(),
            lastWill.GetString(),
            2,
            false);
    }else{
        fprintf(stderr, "Connect failed\n");
    }
}

void my_subscribe_callback(struct mosquitto *mosq, void *userdata, int mid, int qos_count, const int *granted_qos)
{
    int i;

    printf("Subscribed (mid: %d): %d", mid, granted_qos[0]);
    for(i=1; i<qos_count; i++){
        printf(", %d", granted_qos[i]);
    }
    printf("\n");
}

void my_log_callback(struct mosquitto *mosq, void *userdata, int level, const char *str)
{
    /* Print all log messages regardless of level. */
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
    if(!g_mosq){
        fprintf(stderr, "Error: Out of memory.\n");
        return 1;
    }
    mosquitto_log_callback_set(g_mosq, my_log_callback);
    mosquitto_connect_callback_set(g_mosq, my_connect_callback);
    mosquitto_message_callback_set(g_mosq, my_message_callback);
    mosquitto_subscribe_callback_set(g_mosq, my_subscribe_callback);

    if(mosquitto_connect(g_mosq, host, port, keepalive)){
        fprintf(stderr, "Unable to connect.\n");
        return 1;
    }

    mosquitto_loop_forever(g_mosq, -1, 1);

    mosquitto_destroy(g_mosq);
    mosquitto_lib_cleanup();
    return 0;
}
