#include "MqttEventPublisher.h"
#include <mosquitto.h>
#include <iostream>
#include <sstream>

struct MqttEventPublisher::Impl {
    const EnvConfig &config;
    mosquitto *client = nullptr;
    explicit Impl(const EnvConfig &c) : config(c) {}
};

MqttEventPublisher::MqttEventPublisher(const EnvConfig &config) : impl_(std::make_unique<Impl>(config))
{
    mosquitto_lib_init();
    impl_->client = mosquitto_new((config.cameraId + "-daemon").c_str(), true, nullptr);
    if (!impl_->client)
        throw std::runtime_error("cannot create MQTT client");
    mosquitto_username_pw_set(impl_->client, config.mqttUser.c_str(), config.mqttPassword.c_str());
    if (config.mqttTls) {
        if (mosquitto_tls_set(impl_->client, impl_->config.mqttCaFile.c_str(), nullptr, nullptr, nullptr, nullptr) != MOSQ_ERR_SUCCESS)
            throw std::runtime_error("cannot enable MQTT TLS");
        mosquitto_tls_insecure_set(impl_->client, false);
    }
    const auto rc = mosquitto_connect(impl_->client, config.mqttHost.c_str(), config.mqttPort, 30);
    if (rc != MOSQ_ERR_SUCCESS)
        std::cerr << "MQTT connect failed: " << mosquitto_strerror(rc) << '\n';
    else if (mosquitto_loop_start(impl_->client) != MOSQ_ERR_SUCCESS)
        std::cerr << "MQTT network loop failed to start\n";
}

MqttEventPublisher::~MqttEventPublisher()
{
    if (impl_->client) {
        mosquitto_loop_stop(impl_->client, true);
        mosquitto_disconnect(impl_->client);
        mosquitto_destroy(impl_->client);
    }
    mosquitto_lib_cleanup();
}

void MqttEventPublisher::publish(const std::string &event, const PersonDetection &detection)
{
    std::ostringstream payload;
    payload << "{\"event\":\"" << event << "\",\"camera_id\":\""
            << impl_->config.cameraId << "\",\"person_present\":"
            << (detection.personPresent ? "true" : "false")
            << ",\"confidence\":" << detection.confidence << "}";
    if (impl_->client)
        mosquitto_publish(impl_->client, nullptr, impl_->config.mqttTopic.c_str(),
                          static_cast<int>(payload.str().size()), payload.str().c_str(), 1, false);
}
