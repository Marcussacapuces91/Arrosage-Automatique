/**
 * Copyright 2018 Marc SIBERT.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *    http://www.apache.org/licenses/LICENSE-2.0
 * 
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#pragma once

#include <LiquidCrystal.h>

class App {
public:
/**
 * Constructeur de l'application.
 */
  App() :
    lcd(7, 6, 5, 4, 3, 2)
  {}

/**
 * Méthode d'initalisation tardive. 
 * Doit être appelée dans la méthode begin() du programme principal. 
 */
  void begin() {
    uint8_t buf[8];
    
    lcd.begin(16, 2);

    lcd.createChar(0, static_cast<uint8_t*>(memcpy_P(buf, hd, sizeof(buf))));
    lcd.createChar(1, static_cast<uint8_t*>(memcpy_P(buf, bg, sizeof(buf))));
    lcd.clear();

    lcd.print(F("Arrosoir ------")); lcd.write(byte(0));
    lcd.setCursor(0,1);
    lcd.write(byte(1)); lcd.print(F("--- Automatique"));
    delay(1000);

    lcd.createChar(0, static_cast<uint8_t*>(memcpy_P(buf, copy, sizeof(buf))));
    lcd.clear();

    lcd.print('('); lcd.write(byte(0)); lcd.print(F(") Marc SIBERT"));
    lcd.setCursor(0,1);
    lcd.print(F("  "));
    lcd.print(F(__DATE__));
    delay(2000);
    
    lcd.clear();
  }

  float getTemp() const {
    // The internal temperature has to be used
    // with the internal reference of 1.1V.
    // Channel 8 can not be selected with
    // the analogRead function yet.
  
    // Set the internal reference and mux.
    ADMUX = (_BV(REFS1) | _BV(REFS0) | _BV(MUX3));
    ADCSRA |= _BV(ADEN);  // enable the ADC
  
    delay(20);            // wait for voltages to become stable.
  
    ADCSRA |= _BV(ADSC);  // Start the ADC
  
    // Detect end-of-conversion
    while (bit_is_set(ADCSRA,ADSC));
  
    // Reading register "ADCW" takes care of how to read ADCL and ADCH.
    const unsigned wADC = ADCW;
  
    // The offset of 324.31 could be wrong. It is just an indication.
    const float t = (wADC - 324.31 ) / 1.22;
  
    // The returned temperature is in degrees Celsius.
    return t;
  }

/** 
 * Instance LiquidCrystal rendue public pour accéder aux principales fonctions de l'écran.
 */
  LiquidCrystal lcd;

protected:

private:
  static const uint8_t copy[] PROGMEM;
  static const uint8_t hd[] PROGMEM;
  static const uint8_t bg[] PROGMEM;
 
};

static const uint8_t App::copy[] PROGMEM = {
  B01100,
  B10000,
  B10010,
  B01100,
  B00000,
  B00000,
  B00000,
  B00000
};

static const uint8_t App::hd[] PROGMEM = {
  B00000,
  B00000,
  B00000,
  B11110,
  B00011,
  B00011,
  B00011,
  B00011
};

static const uint8_t App::bg[] PROGMEM = {
  B11000,
  B11000,
  B11000,
  B01111,
  B00000,
  B00000,
  B00000,
  B00000
};


/**
 * Instance créée automatiquement de App.
 */ 
App APP;
