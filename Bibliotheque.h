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

class Bibliotheque {
public:
/**
 * Constructeur de l'application.
 */
  Bibliotheque() :
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
    pinMode(RELAY, OUTPUT);

 // Activation du port série.
    Serial.begin(250000);

// Initialisation et affichage d'une bannière sur l'écran LCD.    
    uint8_t buf[8];
    
    lcd.begin(16, 2);

    lcd.createChar(0, static_cast<uint8_t*>(memcpy_P(buf, hd, sizeof(buf))));
    lcd.createChar(1, static_cast<uint8_t*>(memcpy_P(buf, bg, sizeof(buf))));
    lcd.clear();

    lcd.print(F("Arrosoir ------")); lcd.write(byte('\x00'));
    lcd.setCursor(0, 1);
    lcd.write('\x01'); lcd.print(F("___ Automatique"));
    delay(1000);

    lcd.createChar(2, static_cast<uint8_t*>(memcpy_P(buf, copy, sizeof(buf))));
    lcd.clear();

    lcd.print('('); lcd.write('\x02'); lcd.print(F(") Marc SIBERT"));
    lcd.setCursor(0,1);
    lcd.print(F("  "));
    lcd.print(F(__DATE__));
    delay(2000);
    
    lcd.createChar(3, static_cast<uint8_t*>(memcpy_P(buf, b1, sizeof(buf))));
    lcd.createChar(4, static_cast<uint8_t*>(memcpy_P(buf, b2, sizeof(buf))));
    lcd.createChar(5, static_cast<uint8_t*>(memcpy_P(buf, b3, sizeof(buf))));
    lcd.createChar(6, static_cast<uint8_t*>(memcpy_P(buf, b4, sizeof(buf))));
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
      const auto wADC = ADCW;
    
// The offset of 324.31 could be wrong. It is just an indication.
      const float t = (wADC - 324.31f ) / 1.22;
    
// The returned temperature is in degrees Celsius.
      return t;
    } else {
      analogReference(INTERNAL);
      analogRead(LM35_MES);   // throw away first mesure
      delay(20);
      static const byte NB = 20;
      int a = 0;
      for (byte i = NB; i > 0; --i) 
        a += analogRead(LM35_MES) - analogRead(LM35_REF);
      
// 10 mV/°C 0°C == 0mV, valeurs négatives possibles.
      return a * (110.0f/NB/1023);
    }
  }

/**
 * Retourne l'humidité mesurée en % (indication).
 * 
 * @param airValue Valeur maximum retournée par le capteur (à l'air libre).
 * @param waterValue Valeur minimum retournée par le capteur (plongé dans l'eau).
 * @return Une estimation du taux d'humidité en % [0..100].
 */
  byte getMoisture(const unsigned airValue = 520, const unsigned waterValue = 260) const {
    analogReference(DEFAULT);
    analogRead(A2);   // throw away first mesure
    delay(20);
    const unsigned h = analogRead(A2);
    return (h < waterValue) ? 100 : ((h > airValue) ? 0 : 100 - 100 * (h - waterValue) / (airValue - waterValue));
  }

/**
 * Active ou désactive la pompe et la LED.
 * 
 * @param aState État de la pompe après l'appel.
 */
  void setPump(const bool aState) {
    digitalWrite(LED_BUILTIN, aState);
    digitalWrite(RELAY, aState);
  }

/**
 * Imprime un bargraphe de 10 caractères.
 * 
 * @param aPrint un stream de sortie (lcd).
 * @param aValue La valeur à représenter de [0..100].
 */
void barre(const byte aValue) {
  for (int i = 0; i < 10; ++i) {
    if ((i+1) * 10 <= aValue) {
      lcd.write('\xff');
    } else if (i * 10 > aValue) {
      lcd.write(' ');
    } else {
      switch (aValue % 10) {
        case 0 :
        case 1 :
          lcd.write(' ');
          break;
        case 2 :
        case 3 : 
          lcd.write('\x03');
          break;
        case 4 :
        case 5 :
          lcd.write('\x04');
          break;
        case 6 : 
        case 7 : 
          lcd.write('\x05');
          break;
        case 8 :
        case 9 : 
          lcd.write('\x06');
          break;
      }
    }
  }
}

/** 
 * Instance LiquidCrystal rendue public pour accéder aux principales fonctions de l'écran.
 * L'instance est publique pour être accessible en dehors des méthodes de l'application.
 */
  LiquidCrystal lcd;

protected:

/**
 * Définition des ports physique de l'Arduino sur lesquels sont branchés les périphériques.
 */
enum port_t {
// Deux tension du capteur de temp. LM35 + diode.
  LM35_MES = A0,    ///< tension du capteur ;
  LM35_REF = A1,    ///< tension de la diode de 0°C

// Broches de l'afficheur en 4bits + ctrl.
  DISPLAY_RS = 7,   ///< Reset
  DISPLAY_E  = 6,   ///< Enable
  DISPLAY_D7 = 5,   ///< D7, D3
  DISPLAY_D6 = 4,   ///< D6, D2
  DISPLAY_D5 = 3,   ///< D5, D1
  DISPLAY_D4 = 2,   ///< D4, D0

// Capteur d'humidité (analogique).
  SMS_MES = A2,     ///< Soil Moisture Sensor mesure.

// Sortie TOR relai
  RELAY = 12        ///< Relay-Pump output
};

private:
/// Caractère copyright (c)
  static const uint8_t copy[] PROGMEM;
/// Angle haut-droit -| 
  static const uint8_t hd[] PROGMEM;
/// Angle bas-gauche |_ 
  static const uint8_t bg[] PROGMEM;
  
/// 1 barre un pixel |
  static const uint8_t b1[] PROGMEM;
/// 1 barre 2 pixels ||
  static const uint8_t b2[] PROGMEM;
/// 1 barre 3 pixels |||
  static const uint8_t b3[] PROGMEM;
/// 1 barre 4 pixels ||||
  static const uint8_t b4[] PROGMEM;
 
};

static const uint8_t Bibliotheque::copy[] PROGMEM = {
  B01100,
  B10000,
  B10010,
  B01100,
  B00000,
  B00000,
  B00000,
  B00000
};

static const uint8_t Bibliotheque::hd[] PROGMEM = {
  B00000,
  B00000,
  B00000,
  B11100,
  B00010,
  B00001,
  B00001,
  B00001
};

static const uint8_t Bibliotheque::bg[] PROGMEM = {
  B10000,
  B10000,
  B10000,
  B10000,
  B10000,
  B01000,
  B00111,
  B00000
};

static const uint8_t Bibliotheque::b1[] PROGMEM = {
  B10000,
  B10000,
  B10000,
  B10000,
  B10000,
  B10000,
  B10000,
  B10000
};

static const uint8_t Bibliotheque::b2[] PROGMEM = {
  B11000,
  B11000,
  B11000,
  B11000,
  B11000,
  B11000,
  B11000,
  B11000
};

static const uint8_t Bibliotheque::b3[] PROGMEM = {
  B11100,
  B11100,
  B11100,
  B11100,
  B11100,
  B11100,
  B11100,
  B11100
};

static const uint8_t Bibliotheque::b4[] PROGMEM = {
  B11110,
  B11110,
  B11110,
  B11110,
  B11110,
  B11110,
  B11110,
  B11110
};

/**
 * Instance créée automatiquement de Bibliotheque.
 */ 
Bibliotheque Bib;
