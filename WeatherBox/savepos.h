#include <EEPROM.h>

#ifndef _SAVEPOS_H_
// 声明
#define _SAVEPOS_H_

#define EEPROM_POS 1024
#define EEPROM_POS_START 512

void savePos(int16_t dutyCycleSet[3]) {
  EEPROM.begin(EEPROM_POS);
  uint8_t *p = (uint8_t*)(dutyCycleSet);
  for (int i = 0; i < 6; i++) {
    EEPROM.write(i + EEPROM_POS_START, *(p + i));
  }
  EEPROM.commit();
  Serial.println("saved");
}

/*
   获取wifi账号密码信息
*/
int16_t* loadPos() {
  // 为变量请求内存
  int16_t *dutyCycleSet  = new int16_t[3];  
  EEPROM.begin(EEPROM_POS);
  uint8_t *p = (uint8_t*)(dutyCycleSet);
  for (int i = 0; i < 6; i++) {
    *(p + i) = EEPROM.read(i + EEPROM_POS_START);
  }
  EEPROM.commit();
  Serial.println("loaded");
  Serial.print("p1 ");
  Serial.print(dutyCycleSet[0]);
  Serial.print(" p2 ");
  Serial.print(dutyCycleSet[1]);
  Serial.print(" p3 ");
  Serial.println(dutyCycleSet[2]);
  return dutyCycleSet;
}

#endif
