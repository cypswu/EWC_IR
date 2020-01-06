/*
   紅外線發射與接收範例
   開關PIN接地約3秒進入學習6秒，接地不到3秒IR發送
*/

#include <EWC_IR.h>

const byte RECV_PIN = 14;   //接收 an IR detector
const byte SEND_PIN = 15;   //發射 an IR led
const byte beepPin = 13;    //蜂鳴器
const byte buttonPin = 12;  //按鈕 PIN腳(使用內部上拉)
EWC_IR ir(RECV_PIN, SEND_PIN, beepPin);    //收、發、訊號 PIN腳

void setup()  {
  Serial.begin(115200);
  Serial.println();
  pinMode(buttonPin, INPUT_PULLUP);
  ir.begin();
  ir.setDebug(true);
}

void loop()  {
  monitorStatus();      //監測設備狀態
}

/********************************************************************************
   監測設備狀態
*/
void monitorStatus() {
  //開關狀態
  for (int i = 0; i < 100; i++) {
    bool bin = digitalRead(buttonPin);
    if (bin == LOW) {
      if (i == 99) {
        if (ir.saveScanRaw("rt1")) {
          Serial.println("save OK");
        } else {
          Serial.println("fail");
        }
      }
    } else {
      if (i > 3) {
        ir.sendIrRaw("rt1");
      }
      break;
    }
    delay(30);
  }
}
