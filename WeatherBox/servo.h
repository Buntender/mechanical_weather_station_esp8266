#include "savepos.h"

#ifndef _SERVO_H_
// 声明
#define _SERVO_H_

int POS = 0, MID = 1, NEG = 2;

class servo{
  private:
    const int servoMotorPin;
    int curpos = MID;
    int16_t dutyRatioSet[3] = {205, 525, 845};
    int lastangle;
    int dir = 1;
  public:
    servo(int servopin):servoMotorPin(servopin){
     analogWriteFreq(50);  /* Set PWM frequency to 50Hz */
     analogWriteRange(20480);
     
     int16_t* saveddutyRatio = loadPos();
     if(isvaliddutyratio(saveddutyRatio[POS], POS) && isvaliddutyratio(saveddutyRatio[MID], MID) && isvaliddutyratio(saveddutyRatio[NEG], NEG)){
      dutyRatioSet[POS] = saveddutyRatio[POS];
      dutyRatioSet[MID] = saveddutyRatio[MID];
      dutyRatioSet[NEG] = saveddutyRatio[NEG];
     }
     else savePos(dutyRatioSet);
     Serial.println("init");
     analogWrite(servoMotorPin, 2048 - dutyRatioSet[curpos]);  /* Write duty cycle to pin */
     delay(3000);
     lastangle = analogRead(A0);
     Serial.println("init finished");
     Serial.print("dutyRatio ");
     Serial.print(dutyRatioSet[curpos]);
     Serial.print(" curangle ");
     Serial.println(lastangle);
     analogWrite(servoMotorPin, 0);  /* Write duty cycle to pin */
    }

    int isvalidangle(int angle, int pos){
      return (angle > 150 && angle < 350 && pos == POS) || (angle > 450 && angle < 650 && pos == MID) || (angle > 750 && angle < 950 && pos == NEG);
    }

    int isvaliddutyratio(int dutycycle, int pos){
      return (dutycycle > 110 && dutycycle < 310 && pos == POS) || (dutycycle > 420 && dutycycle < 620 && pos == MID) || (dutycycle > 750 && dutycycle < 950 && pos == NEG);
    }
    
    float tanhnonlinear(float t){
      return 0.5 + 0.5 * tanh(4 * t - 2) / 0.964;
    }
    
    float bouncenonlinear(float t){
//      return 5*pow(t, 4)-12*pow(t, 3)+8*pow(t, 2);
      return 6*pow(t, 4)-14*pow(t, 3)+9*pow(t, 2);
    }

    int adjustto(int pos, int angle){
      int dutycycle = dutyRatioSet[pos];

      Serial.println("adjusting");
      
      analogWrite(servoMotorPin, 2048 - dutycycle);
      delay(500);
      while(analogRead(A0) > angle && isvaliddutyratio(dutycycle, pos)){
        dutycycle-=5;
        analogWrite(servoMotorPin, 2048 - dutycycle);
        delay(100);
      }
      while(analogRead(A0) < angle && isvaliddutyratio(dutycycle, pos)){
        dutycycle+=5;
        analogWrite(servoMotorPin, 2048 - dutycycle);
        delay(100);
      }
      analogWrite(servoMotorPin, 2048 - dutycycle);
      delay(300);

      while(analogRead(A0) > angle && isvaliddutyratio(dutycycle, pos)){
        dutycycle--;
        analogWrite(servoMotorPin, 2048 - dutycycle);
        delay(60);
      }
      while(analogRead(A0) < angle && isvaliddutyratio(dutycycle, pos)){
        dutycycle++;
        analogWrite(servoMotorPin, 2048 - dutycycle);
        delay(60);
      }
      
      if(!isvaliddutyratio(dutycycle, pos)){
        dutycycle = dutyRatioSet[pos];
        analogWrite(servoMotorPin, 2048 - dutycycle);
        delay(500);
        Serial.println("invalid");
      }
      analogWrite(servoMotorPin, 0);
      Serial.print("Dutycycle for ");
      Serial.print(angle);
      Serial.print(" is ");
      Serial.println(dutycycle);
      return dutycycle;
    }

    
    void trans(int dstpos){
      int curangle = analogRead(A0);
      if(dstpos > 2 || dstpos < 0) return;
      if(curangle - lastangle > 5 || curangle - lastangle < -5){
        if(isvalidangle(curangle, curpos)){
          dutyRatioSet[curpos] = adjustto(curpos, curangle);
          savePos(dutyRatioSet);
        }
        else{
          analogWrite(servoMotorPin, 2048 - dutyRatioSet[curpos]);
          delay(1000);
          analogWrite(servoMotorPin, 0);
        }
      }
      Serial.print("curangle - lastangle ");
      Serial.println(curangle - lastangle);
      if(dstpos == curpos) return;
      int dstDutyCycle = dutyRatioSet[dstpos], curDutyCycle = dutyRatioSet[curpos];
      
      float interval = 1 / (15 + abs(dstDutyCycle - curDutyCycle) * 0.05);
      
      Serial.print("dstDutyCycle ");
      Serial.print(dstDutyCycle);
      Serial.print(" curDutyCycle ");
      Serial.println(curDutyCycle);
      for(float t = 0; t <= 1; t+=interval){
        analogWrite(servoMotorPin, 2048 - curDutyCycle - (dstDutyCycle - curDutyCycle) * tanhnonlinear(t));  /* Write duty cycle to pin */
        delay(20);
      }
      curpos = dstpos;
      analogWrite(servoMotorPin, curpos);
      delay(200);
      analogWrite(servoMotorPin, 0);
      delay(200);
      lastangle = analogRead(A0);
    }

    void stepforward(){
      if(curpos <= POS) dir = 1;
      if(curpos >= NEG) dir = -1;
      trans(curpos + dir);
    }
};

#endif
