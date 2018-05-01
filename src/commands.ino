/* ================================================================
      Handler for commmands received through subscripted topic
   ================================================================ */
void control(String command, String arg1, String arg2, String arg3) {
        if (command == "setBrightness") {
                setBrightness(arg1);
        }
        else if (command == "setColor") {
                setColor(arg1, arg2, arg3);
        }
        else if (command == "togglePower") {
                togglePower();
        }
        else if (command == "toggleRainbow") {
                toggleRainbow();
        }
        else {
                Serial.println("Command undefined");
        }
}

/* ================================================================
                        Command functions
   ================================================================ */

void setBrightness(String brightness){
        FastLED.setBrightness(brightness.toInt());
        FastLED.show();
}

void setColor(String r, String g, String b){
        if(isLedOn) {
                isRainbow = false;
                //FastLED.clear();
                for(int i=0; i<NUM_LEDS; i++) {
                        R = r.toInt();
                        G = g.toInt();
                        B = b.toInt();
                        leds[i].setRGB(R,G,B);
                        FastLED.delay(30);
                        FastLED.show();
                }
        }
}

void togglePower(){
        if(isLedOn) {
                ledOff();
        }
        else {
                ledOn();
        }
}

void toggleRainbow(){
        if(isRainbow) {
                isRainbow = false;
                ledOff();
        }
        else {
                isRainbow = true;
                ledOn();
        }
}

void ledOff(){
        isLedOn = false;
        for(int i=0; i<NUM_LEDS; i++) {
                leds[i].setRGB(0, 0, 0);
                FastLED.delay(30);
                FastLED.show();
        }
}

void ledOn(){
        isLedOn = true;
        for(int i=0; i<NUM_LEDS; i++) {
                leds[i].setRGB(R,G,B);
                FastLED.delay(30);
                FastLED.show();
        }
}

void showRainbow(){
        if(isLedOn) {
                // draw a generic, no-name rainbow
                static uint8_t starthue = 0;
                fill_rainbow( leds + 5, NUM_LEDS - 5, --starthue, 20);

                // Choose which 'color temperature' profile to enable.
                uint8_t secs = (millis() / 1000) % (DISPLAYTIME * 2);
                if( secs < DISPLAYTIME) {
                        FastLED.setTemperature( TEMPERATURE_1 ); // first temperature
                        leds[0] = TEMPERATURE_1; // show indicator pixel
                } else {
                        FastLED.setTemperature( TEMPERATURE_2 ); // second temperature
                        leds[0] = TEMPERATURE_2; // show indicator pixel
                }
                FastLED.delay(8);
                FastLED.show();
        }
}
