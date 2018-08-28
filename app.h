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
    lcd(DISPLAY_RS, DISPLAY_E, DISPLAY_D7, DISPLAY_D6, DISPLAY_D5, DISPLAY_D4)
  {}

/**
 * Méthode d'initalisation tardive. 
 * Doit être appelée dans la méthode begin() du programme principal. 
 */
  void begin() {

 // Définition des états des broches
    pinMode(LM35_MES, INPUT);
    pinMode(LM35_REF, INPUT);
    pinMode(SMS_MES, INPUT);

    pinMode(LED_BUILTIN, OUTPUT);

 // Activation du port série.
    Serial.begin(250000);

// Initialisation et affichage d'une bannière sur l'écran LCD.    
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

/**
 * Retourne la température mesurée par le LM35 ou le capteur interne de l'ATMega.
 * 
 * @param w Designe le capteur à utiliser (0 pour LM35 par défaut, ou INTERNAL pour la pude du contrôleur).
 * @return Un nombre réel indiquant la température en °C.
 */
  float getTemp(const byte w = 0) const {
    if (INTERNAL == w) {
// Following code is copyright Arduino and come from http://playground.arduino.cc/Main/InternalTemperatureSensor
// The internal temperature has to be used with the internal reference of 1.1V.
// Channel 8 can not be selected with the analogRead function yet.
  
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
    } else {
      analogReference(INTERNAL);
      analogRead(LM35_MES);   // throw away first mesure
      delay(20);
      const unsigned a0 = analogRead(LM35_MES);
      const unsigned a1 = analogRead(LM35_REF);
      
// 10 mV/°C 0°C == 0mV, valeurs négatives possibles.
      return (a0-a1) * 110 / 1023.0f;
    }
  }

  byte getMoisture(const unsigned airValue = 520, const unsigned waterValue = 260) const {
    analogReference(DEFAULT);
    analogRead(A2);   // throw away first mesure
    delay(20);
    const unsigned h = analogRead(A2);
    return (h < waterValue) ? 100 : ((h > airValue) ? 0 : 100 - 100 * (h - waterValue) / (airValue - waterValue));
  }

/** 
 * Instance LiquidCrystal rendue public pour accéder aux principales fonctions de l'écran.
 */
  LiquidCrystal lcd;

protected:

enum port_t {
  LM35_MES = A0,
  LM35_REF = A1,
  
  DISPLAY_RS = 7,
  DISPLAY_E  = 6,
  DISPLAY_D7 = 5,
  DISPLAY_D6 = 4,
  DISPLAY_D5 = 3,
  DISPLAY_D4 = 2,

  SMS_MES = A2      ///< Soil Moisture Sensor mesure.
};

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
  B11100,
  B00010,
  B00001,
  B00001,
  B00001
};

static const uint8_t App::bg[] PROGMEM = {
  B10000,
  B10000,
  B01000,
  B00111,
  B00000,
  B00000,
  B00000,
  B00000
};


/**
 * Instance créée automatiquement de App.
 */ 
App APP;
