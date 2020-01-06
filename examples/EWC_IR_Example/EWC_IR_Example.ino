/*
   紅外線發射與接收範例
   兩聲 → 過1秒一聲 → 開始接收 → 兩聲 → 結束接收 → 過5秒一聲 → 發射 → 等待10秒
*/

#include <EWC_IR.h>

EWC_IR ir(14, 15, 13);  //收、發、訊號 PIN腳

void setup()  {
  Serial.begin(115200);
  Serial.println();
  ir.begin();
  ir.setDebug(true);
}

void loop()  {
  ir.beeEnd();
  delay(1000);
  if (ir.saveScanRaw("rt1")) {
    Serial.println("save OK");
  } else {
    Serial.println("fail");
  }
  delay(5000);
  ir.beeStart();
  ir.sendIrRaw("rt1");
  delay(10000);
}

