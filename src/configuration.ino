/* ================================================================
                  Load configuration from SPIFFS
   ================================================================ */
bool loadConfig() {
        File configFile = SPIFFS.open("/config.json", "r");
        if (!configFile) {
                Serial.println("Failed to open config file");
                return false;
        }

        size_t size = configFile.size();
        if (size > 4096) {
                Serial.println("Config file size is too large");
                return false;
        }

        // Allocate a buffer to store contents of the file.
        std::unique_ptr<char[]> buf(new char[size]);

        // We don't use * here because ArduinoJson library requires the input
        // buffer to be mutable. If you don't use ArduinoJson, you may as well
        // use configFile.readBytes instead.
        configFile.readBytes(buf.get(), size);

        // See this helper assistant to calculate the bufferSize - https://bblanchon.github.io/ArduinoJson/assistant/
        const size_t bufferSize = JSON_OBJECT_SIZE(1) + JSON_OBJECT_SIZE(2) + 3 * JSON_OBJECT_SIZE(3) + JSON_OBJECT_SIZE(4) + JSON_OBJECT_SIZE(7) + JSON_OBJECT_SIZE(9) + 480;
        StaticJsonBuffer<bufferSize> jsonBuffer;
        JsonObject& root = jsonBuffer.parseObject(buf.get());

        if (!root.success()) {
                Serial.println("Failed to parse config file");
                return false;
        }

        configData.name = root["name"].as<String>();
        // WiFi
        JsonObject&  wifi = root["wifi"];
        configData.wifi_ssid = wifi["ssid"].as<String>();
        configData.wifi_password = wifi["password"].as<String>();
        // MQTT
        JsonObject&  mqtt = root["mqtt"];
        configData.mqtt_server = mqtt["server"].as<String>();
        configData.mqtt_port = mqtt["port"];
        configData.mqtt_username = mqtt["username"].as<String>();
        configData.mqtt_password = mqtt["password"].as<String>();
        configData.mqtt_topic_publish = mqtt["topic_publish"].as<String>();
        configData.mqtt_topic_subscribe = mqtt["topic_subscribe"].as<String>();
        configData.mqtt_topic_metadata = mqtt["topic_metadata"].as<String>();

        return true;
}
