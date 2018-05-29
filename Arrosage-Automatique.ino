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

#include "app.h"

void setup() {
  APP.begin();
  APP.lcd.print(F("Bonjour monde !"));
}

void loop() {
  APP.lcd.setCursor(0,1);
  APP.lcd.print(millis());
  APP.lcd.print(F("ms "));
  APP.lcd.print(APP.getTemp(),1);
  APP.lcd.print(char(223));
  APP.lcd.print('C');
}
