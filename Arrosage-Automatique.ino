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

#include "Bibliotheque.h"
#include <IRremote.h>

int RECV_PIN = 11;
IRrecv irrecv(RECV_PIN);

/**
 * Affichage des informations courantes sur 2 lignes.
 * 
 * @param aApp un lien vers l'instance de l'application.
 * @param aTemp La température courante en °C pour un affichage à une décimale.
 * @param aHumi Le poucentage d'humidité pour un affichage en bargraph.
 */
void affichageDefaut(const float& aTemp, const byte aHumi, const bool aPump = false) {
  Bib.lcd.setCursor(0,0);
  Bib.lcd.print(F("Temp: "));
  Bib.lcd.print(aTemp,1);
  Bib.lcd.write('\xdf'); // char(223) = '°'
  Bib.lcd.print(F("C "));

  Bib.lcd.setCursor(15,0);
  Bib.lcd.print(aPump ? '\xff' : ' ');

  Bib.lcd.setCursor(0,1);
  Bib.lcd.print(F("Humi: "));
  Bib.barre(aHumi);
}

void affichageConsigne(const byte aConsigne, const unsigned aDelay = 2000) {
  Bib.lcd.setCursor(0,0);
  Bib.lcd.print(F("Consigne : "));
  Bib.lcd.print(aConsigne);
  
  Bib.lcd.setCursor(0,1);
  Bib.lcd.print(F(" > "));
  Bib.barre(aConsigne);
  Bib.lcd.print(F(" <"));
  delay(aDelay);
}

bool decodeIR(byte& aConsigne) {
  decode_results results;

  if (!irrecv.decode(&results) ) return false;
  if (results.value == REPEAT) {
    irrecv.resume(); // Receive the next value
    return false;
  }

  switch (results.value) {
    case 0xFF6897 : // '0'
      aConsigne = 0;
      break;
    case 0xFF30CF : // '1'
      aConsigne = 10;
      break;
    case 0xFF18E7 : // '2'
      aConsigne = 20;
      break;
    case 0xFF7A85 : // '3'
      aConsigne = 30;
      break;
    case 0xFF10EF : // '4'
      aConsigne = 40;
      break;
    case 0xFF38C7 : // '5'
      aConsigne = 50;
      break;
    case 0xFF5AA5 : // '6'
      aConsigne = 60;
      break;
    case 0xFF42BD : // '7'
      aConsigne = 70;
      break;
    case 0xFF4AB5 : // '8'
      aConsigne = 80;
      break;
    case 0xFF52AD : // '9'
      aConsigne = 90;
      break;
    case 0xFF9867 : // '100+'
      aConsigne = 100;
      break;
    case 0xFFA857 : // '+'
      if (aConsigne < 100) aConsigne += 1;
      break;
    case 0xFFE01F : // '-'
      if (aConsigne > 0) aConsigne -= 1;
      break;
    case 0xFF22DD : // '|<<'
      aConsigne = aConsigne > 5 ? aConsigne - 5 : 0;
      break;
    case 0xFF02FD : // '>>|'
      aConsigne = aConsigne < 95 ? aConsigne + 5 : 100;
      break;
    default :
      Serial.print(F("Value : "));
      Serial.print(results.value, HEX);
      Serial.print(F(" ("));
      Serial.print(results.bits, DEC);
      Serial.println(F(" bits)"));
      irrecv.resume(); // Receive the next value
      return false;
  }

  irrecv.resume(); // Receive the next value
  return true;
}

byte consigne = 0;

/**
 * Fonction de démarrage (exécutée une seule fois).
 */
void setup() {
  Bib.begin();
  irrecv.enableIRIn(); // Start the receiver
}

/**
 * Régulation TOR (seuil)
 */
void loop1() {

  if ( decodeIR(consigne) ) affichageConsigne(consigne);

  const auto s = millis();
  if ((s % 500) > 0) return;  // boucle tous les 1/2 de sec

  const auto temp = Bib.getTemp();
  const auto humi = Bib.getMoisture();
  const bool pompe = (humi < consigne) && (temp < 30);

  Bib.setPump(pompe);
  affichageDefaut(temp, humi, pompe);

  Serial.print(consigne);
  Serial.print(',');
  Serial.print(humi);
  Serial.print(',');
  Serial.println(pompe ? 1 : 0);
 
}

/**
 * Régulation proportionnelle.
 * 
 * Durée d'arrosage = P * (consigne - mesure)
 */
void loop2() {

  if ( decodeIR(consigne) ) affichageConsigne(consigne);

  const auto s = millis();
  if ((s % 500) > 0) return;  // boucle tous les 1/4 de sec

  const auto temp = Bib.getTemp();
  const auto humi = Bib.getMoisture();
  const int pompe = temp < 30 ? 10 * (consigne - humi) : 0;

  if (pompe < 0) {
    Bib.setPump(false);
  } else if (pompe > 450) {
    Bib.setPump(true);
  } else {
    Bib.setPump(true);
    delay(pompe);
    Bib.setPump(false);
  }
  
  affichageDefaut(temp, humi);

  Serial.print(consigne);
  Serial.print(',');
  Serial.print(humi);
  Serial.print(',');
  Serial.println(pompe);
 
}

/**
 * Régulation proportionnelle.
 * 
 * Durée d'arrosage = P * Somme(consigne - mesure)
 */
void loop() {
  static long somme = 0;
  if ( decodeIR(consigne) ) affichageConsigne(consigne);

  const auto s = millis();
  if ((s % 500) > 0) return;  // boucle tous les 1/4 de sec

  const auto temp = Bib.getTemp();
  const auto humi = Bib.getMoisture();
  somme += consigne - humi;
  const int pompe = 1 * somme;

  if (pompe < 0 || temp > 30) {
    Bib.setPump(false);
  } else if (pompe > 450) {
    Bib.setPump(true);
  } else {
    Bib.setPump(true);
    delay(pompe);
    Bib.setPump(false);
  }
  
  affichageDefaut(temp, humi);

  Serial.print(consigne);
  Serial.print(',');
  Serial.print(humi);
  Serial.print(',');
  Serial.println(pompe);
 
}
