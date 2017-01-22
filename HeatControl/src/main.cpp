#include "stdafx.h"

#include <mosquittopp.h>
#include <iostream>

class MosqClient: public mosqpp::mosquittopp
{
public:
    using mosqpp::mosquittopp::mosquittopp;

private:
    virtual void on_connect(int rc);
    virtual void on_disconnect(int rc);
    virtual void on_publish(int mid);
    virtual void on_message(const struct mosquitto_message *message);
    virtual void on_subscribe(int mid, int qos_count, const int *granted_qos);
    virtual void on_unsubscribe(int mid);
    virtual void on_log(int level, const char *str);;
    virtual void on_error();
};

int main()
{
    mosqpp::lib_init();

    //char* host = "test.mosquitto.org";
    char* host = "localhost";
    int port = 1883;
    int keepalive = 60;
    bool clean_session = true;

    mosqpp::mosquittopp runner("HeatControl", clean_session);

    if (runner.connect(host, port, keepalive)) {
        std::cout << "Unable to connect.\n";
        return 1;
    }
    runner.loop_forever(keepalive);

    return mosqpp::lib_cleanup();
}

void MosqClient::on_connect(int rc) {
    if (!rc) {
        /* Subscribe to broker information topics on successful connect. */
        std::cout << "Connect successfull, subscribing." << std::endl;
        subscribe(nullptr, "$SYS/#", 2);
    }
    else {
        std::cout << "Connect failed" << std::endl;
    }
}

void MosqClient::on_disconnect(int rc) {
    return; 
}

void MosqClient::on_publish(int mid) { 
    return; 
}

void MosqClient::on_message(const mosquitto_message * message) {
    if (message->payloadlen) {
        std::cout << message->topic << " " << message->payload;
    }
    else {
        std::cout << message->topic << " (null)" << std::endl;
    }
    fflush(stdout);
}

void MosqClient::on_subscribe(int messageId, int qos_count, const int * granted_qos) {
    std::cout << "Subscribed (mid: " << messageId << " : " << granted_qos[0];
    for (int i = 1; i<qos_count; i++) {
        std::cout << ", " << granted_qos[i];
    }
    std::cout << std::endl;
    return;
}

void MosqClient::on_unsubscribe(int mid) { 
    return; 
}

void MosqClient::on_log(int level, const char * str) {
    std::cout << "log: " << str << std::endl;
}

void MosqClient::on_error() {
    return; 
}
