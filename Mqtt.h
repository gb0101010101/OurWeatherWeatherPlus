/*
 * Mqtt.h
 *
 *  Created on: Mar 21, 2019
 *      Author: greg
 */
#include <WiFiClient.h>
#include <PubSubClient.h>
#include "vector"

std::vector<String> topics;
std::vector<bool> topics_enabled;

WiFiClient wifi_client;
PubSubClient mqtt_client(wifi_client);

// Setup default variables.

// Set the IP address of the MQTT Server
// e.g. 129, 168, 1, 50
IPAddress mqtt_server(000, 000, 000, 000);
char* mqtt_user = "";
char* mqtt_pass = "";
uint16_t mqtt_port = 1883;

// For now set this to true when values above
// have been configured to start MQTT.
bool mqtt_configured = false;

// True when MQTT has connected during setup.
bool mqtt_valid = false;

// Controls format of data sent
// True - All values will be sent as one JSON string to one topic.
// False - Each piece of data will be sent as separate topic.

// TODO: DO NOT USE 'true' option for now as there are buffer issues.
bool mqtt_send_single = false;

// When sending a single topic the buffer may be too small.
// Increase it by un-commenting the following.
// #define MQTT_MAX_PACKET_SIZE 1024

void mqttCallback(char* topic, byte* payload, unsigned int length) {
  // handle message arrived
}

void mqttReconnect() {
  if (mqtt_client.connect(stationName.c_str(), mqtt_user, mqtt_pass)) {
    Serial.println("MQTT: reconnection successful.");
    mqtt_valid = true;
  } else {
    Serial.println(
        "MQTT: reconnection failed " + String(mqtt_client.state()) + ".");
  }
}

void mqttSetup() {
  if (mqtt_configured) {
    mqtt_client.setServer(mqtt_server, mqtt_port);
    mqtt_client.setCallback(mqttCallback);
    if (mqtt_client.connect(stationName.c_str(), mqtt_user, mqtt_pass)) {
      Serial.println("MQTT: connection successful.");
      mqtt_valid = true;
    } else {
      Serial.println(
          "MQTT: connection failed " + String(mqtt_client.state()) + ".");
    }

    // Populate topics vector.
    // AM2315_Temperature
    topics.push_back("AM2315/Temperature");
    topics_enabled.push_back(AM2315_Present);
    // AM2315_Humidity
    topics.push_back("AM2315/Humidity");
    topics_enabled.push_back(AM2315_Present);
    // BMP180_Temperature
    topics.push_back("BMP180/Temperature");
    topics_enabled.push_back(BMP180Found || BMP280Found);
    // BMP180_Pressure
    topics.push_back("BMP180/Pressure");
    topics_enabled.push_back(BMP180Found || BMP280Found);
    // BMP180_Altitude
    topics.push_back("BMP180/Altitude");
    topics_enabled.push_back(BMP180Found || BMP280Found);
    // currentWindSpeed
    topics.push_back("Wind/Speed/Current");
    topics_enabled.push_back(true);
    // currentWindGust
    topics.push_back("Wind/Gust/Current");
    topics_enabled.push_back(true);
    // currentWindDirection
    topics.push_back("Wind/Direction/Current");
    topics_enabled.push_back(true);
    // rainTotal
    topics.push_back("Rain/Total");
    topics_enabled.push_back(true);
    // windSpeedMin
    topics.push_back("Wind/Speed/Minimum");
    topics_enabled.push_back(true);
    // windSpeedMax
    topics.push_back("Wind/Speed/Maximum");
    topics_enabled.push_back(true);
    // windGustMin
    topics.push_back("Wind/Gust/Minimum");
    topics_enabled.push_back(true);
    // windGustMax
    topics.push_back("Wind/Gust/Maximum");
    topics_enabled.push_back(true);
    // windDirectionMin
    topics.push_back("Wind/Direction/Minimum");
    topics_enabled.push_back(true);
    // windDirectionMax
    topics.push_back("Wind/Direction/Maximum");
    topics_enabled.push_back(true);
    // EnglishOrMetric
    topics.push_back("Station/Units");
    topics_enabled.push_back(true);
    // currentTimeString
    topics.push_back("Station/Time");
    topics_enabled.push_back(true);
    // stationName
    topics.push_back("Station/Name");
    topics_enabled.push_back(true);
    // currentAirQualitySensor
    topics.push_back("AirQuality/Sensor");
    topics_enabled.push_back(AirQualityPresent);
    // currentAirQuality
    topics.push_back("AirQuality/Reading");
    topics_enabled.push_back(AirQualityPresent);
    // BatteryVoltage
    topics.push_back("Battery/Voltage");
    topics_enabled.push_back(AirQualityPresent);
    // BatteryCurrent
    topics.push_back("Battery/Current");
    topics_enabled.push_back(AirQualityPresent);
    // SolarPanelVoltage
    topics.push_back("Solar/Voltage");
    topics_enabled.push_back(AirQualityPresent);
    // SolarPanelCurrent
    topics.push_back("Solar/Current");
    topics_enabled.push_back(AirQualityPresent);
    // LoadVoltage
    topics.push_back("Load/Voltage");
    topics_enabled.push_back(AirQualityPresent);
    // LoadCurrent
    topics.push_back("Load/Current");
    topics_enabled.push_back(AirQualityPresent);
    // WXBatteryVoltage
    topics.push_back("WX/Battery/Voltage");
    topics_enabled.push_back(WXLink_Present);
    // WXBatteryCurrent
    topics.push_back("WX/Battery/Current");
    topics_enabled.push_back(WXLink_Present);
    // WXSolarPanelVoltage
    topics.push_back("WX/Solar/Voltage");
    topics_enabled.push_back(WXLink_Present);
    // WXSolarPanelCurrent
    topics.push_back("WX/Solar/Current");
    topics_enabled.push_back(WXLink_Present);
    // 0.00
    topics.push_back("NotSet");
    topics_enabled.push_back(false);
    // WXLoadCurrent
    topics.push_back("WX/Load/Current");
    topics_enabled.push_back(WXLink_Present);
    // invalidTemperatureFound
    topics.push_back("Debug/TemperatureValid");
    topics_enabled.push_back(AM2315_Present);
    // WXLastMessageGood
    topics.push_back("Debug/WXMessageValid");
    topics_enabled.push_back(WXLink_Present);
    // pubNubEnabled
    topics.push_back("Station/PubNub");
    topics_enabled.push_back(pubNubEnabled);
    // as3935_LastLightning
    topics.push_back("Lightning/Last/Full");
    topics_enabled.push_back(AS3935Present);
    // as3935_LastLightningTimeStamp
    topics.push_back("Lightning/Last/Time");
    topics_enabled.push_back(AS3935Present);
    // as3935_LastLightningDistance
    topics.push_back("Lightning/Last/Distance");
    topics_enabled.push_back(AS3935Present);
    // as3935_LastEvent
    topics.push_back("Lightning/Event/Last");
    topics_enabled.push_back(AS3935Present);
    // as3935_LastEventTimeStamp
    topics.push_back("Lightning/Event/Time");
    topics_enabled.push_back(AS3935Present);
    // as3835_LightningCountSinceBootup
    topics.push_back("Lightning/Strikes");
    topics_enabled.push_back(AS3935Present);
  }
}

/*
 * Send all data using MQTT
 *
 * This is a quick hack using exist REST full data string.
 */
int mqttSend(String data_string) {
  if (mqtt_valid) {
    if (!mqtt_client.connected()) {
      mqttReconnect();
    }

    std::vector<String> values;

    String temp_string;
    char delimeter = ',';
    char last_char;
    int data_length = data_string.length();
    Serial.println("MQTT: Data length: " + String(data_length));
    for (int i = 0; i < data_length + 1; i++) {
      last_char = data_string.charAt(i);
      if (last_char == delimeter) {
        values.push_back(temp_string);
        temp_string = "";
      } else {
        temp_string += String(last_char);
      }
      if (i == data_length && temp_string != "") {
        values.push_back(temp_string);
      }
    }

    int topics_size = topics.size();
    int values_size = values.size();

    if (topics_size != values_size) {
      Serial.println(
          "MQTT: ERROR: Data size mismatch. Topics=" + String(topics.size())
              + " Values=" + String(values.size()));
      Serial.println(data_string);
      String data_debug;
      for (int i = 0; i < values_size; i++) {
        data_debug += values[i];
        if (i < values_size) {
          data_debug += ",";
        }
      }
      Serial.println(data_debug);

    } else {
      Serial.println("MQTT: Sending.");
      if (mqtt_send_single) {
        String json = "{";
        for (int i = 0; i < topics_size; i++) {
          if (topics_enabled[i]) {
            String label = topics[i];
            label.replace("/", "_");
            json += label + ":" + values[i];
            if (i < topics_size) {
              json += ",";
            }
          }
        }
        json += "}";
        String topic = "/" + stationName + "/State/All";
        mqtt_client.publish(topic.c_str(), json.c_str());
#ifdef DEBUG
        Serial.println(json);
        Serial.println(json.length());
#endif
      } else {
        for (int i = 0; i < topics_size; i++) {
          if (topics_enabled[i]) {
            String topic = "/" + stationName + "/State/" + topics[i];
            mqtt_client.publish(topic.c_str(), values[i].c_str());
          }
        }
      }
      // Add a short delay after sending as sometimes MQTT fails to publish.
      delay(100);
      return 1;
    }
  }
  return 0;
}
