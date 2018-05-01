/* ================================================================
                          MQTT functions
   ================================================================ */
void connectMQTT() {
        // Loop until we are reconnected
        while (!client.connected()) {
                Serial.print("Attempting MQTT connection... ");
                // Attempting to connect
                if (client.connect((const char*) clientID, configData.mqtt_username.c_str(), configData.mqtt_password.c_str(), configData.mqtt_topic_metadata.c_str(), 1, 1, String("{\"name\": \"" + configData.name + "\", \"status\": \"disconnected\"}").c_str())) {
                        Serial.println("\t/!\\ connected /!\\");
                        // Sending log "connected" to broker
                        client.publish(configData.mqtt_topic_metadata.c_str(), String("{\"name\": \"" + configData.name + "\", \"status\": \"connected\"}").c_str(), true);
                        client.subscribe(configData.mqtt_topic_subscribe.c_str(), 1);
                } else {
                        Serial.print("failed, rc=");
                        Serial.print(client.state());
                        Serial.println(" try again in 5 seconds");
                        // Try to reconnect to WiFi network
                        if(WiFi.status() != WL_CONNECTED) {
                                WiFi.begin(configData.wifi_ssid.c_str(), configData.wifi_password.c_str());
                                while (WiFi.status() != WL_CONNECTED) {
                                        delay(500);
                                        Serial.print(".");
                                }
                        }
                        // Wait 5 seconds before retrying
                        delay(5000);
                }
        }
}

/*
   void sendMQTT() {
        client.publish(configData.mqtt_topic.c_str(), String("{\"qx\": " + String(qx, 3) + ", \"qy\": " + String(qy, 3) + ", \"qz\": " + String(qz, 3) + ", \"qw\": " + String(qw, 3) +"}").c_str(), false);
   }
 */

// handles message arrived on subscribed topic(s)
void callback(char* topic, byte* payload, unsigned int length) {
        char json[150];
        Serial.print("Message arrived ["); Serial.print(topic); Serial.print("] ");
        for (int i = 0; i < length; i++) {
                Serial.print((char)payload[i]);
                json[i] = (char)payload[i];
        }
        Serial.println();
        Serial.println(json);

        StaticJsonBuffer<200> jsonBuffer;
        JsonObject& root = jsonBuffer.parseObject(json);
        if (!root.success()) {
                Serial.println("parseObject() failed");
                return;
        }
        String command = root["command"];
        String arg1 = root["arg1"];
        String arg2 = root["arg2"];
        String arg3 = root["arg3"];
        control(command, arg1, arg2, arg3);
}
