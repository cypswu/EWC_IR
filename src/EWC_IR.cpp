/** EWC IoT INC.
	Copyright (c) 2017 by lung@ewciot.com
	FileName: EWC_IR.cpp
	Create: 2017.11.15 lung@ewciot.com
	UpDate: 2017.11.15
	Last Version: 1.0.0
	Class: EWC_IR
	Description: 意微控紅外線學習與發射控制物件
	Modification:
	Update		By  		Description
	==========	==========	==============================
	2017.11.15  lung		建立意微控紅外線收發物件

	MEMO:
	  宣告紅外線發射與接收的接腳，簡單進行紅外線學習與發射，並且可以使用揚聲器或燈號來顯示狀態
	  
	X	期望的項目
	==	============================================================
		●宣告紅外線發射與接收的接腳，簡單進行紅外線學習與發射
*/

#include "EWC_IR.h"


/********************************************************************************
   初始化設定紅外線發射與接收的PIN腳
*/
EWC_IR::EWC_IR(byte rpin, byte spin) {
	RECV_PIN = rpin;
	SEND_PIN = spin;
}
EWC_IR::EWC_IR(int rpin, int spin) {
	RECV_PIN = (byte) rpin;
	SEND_PIN = (byte) spin;
}
/********************************************************************************
   初始化設定【紅外線發射PIN腳】與【接收的PIN腳】與【蜂鳴器PIN腳】
*/
EWC_IR::EWC_IR(byte rpin, byte spin, byte beePin) {
	RECV_PIN = rpin;
	SEND_PIN = spin;
	SIGN_PIN = beePin;
}
EWC_IR::EWC_IR(int rpin, int spin, int beePin) {
	RECV_PIN = (byte) rpin;
	SEND_PIN = (byte) spin;
	SIGN_PIN = (byte) beePin;
}

EWC_IR::~EWC_IR(void) {
}

//*******************************************************************************
////// Public: //////
//*******************************************************************************

bool EWC_IR::begin() {
	if (SIGN_PIN != 255)
		pinMode(SIGN_PIN, OUTPUT);
	if (SEND_PIN != 255)
		pinMode(SEND_PIN, OUTPUT);
	if (RECV_PIN != 255)
		pinMode(RECV_PIN, INPUT);
	// 啟用SPIFFS檔案系統
	if (!SPIFFS.begin()) {
		DEBUG_IR("[SPIFFS] error : SPIFFS Mount Failed");
		return false;
	} else {
		return true;
	}
}

void EWC_IR::end() {
	// 關閉SPIFFS檔案系統
	SPIFFS.end();
	DEBUG_IR("SPIFFS END.");
}

// 設定蜂鳴器或燈號接腳
void EWC_IR::setSign(byte beePIN) {
	SIGN_PIN = beePIN;
}
void EWC_IR::setSign(int beePIN) {
	SIGN_PIN = beePIN;
}

void EWC_IR::setDebug(bool isdbg){
	isDebug = isdbg;
}

//設定紅外線保存路徑
void EWC_IR::setFilePath(String path) {
	INFO_PATH = path;
}
	
// 列印紅外線 Raw data
void EWC_IR::printRaw() {
	DEBUG_TAG();
	DEBUG(String((int) ir_x));
	DEBUG("::");
	for (int i = 0; i < ir_x ; i++) {
		if (i > 0) {
		DEBUG(String(irBuffer[i] - irBuffer[i - 1]));
		DEBUG(",");
		}
	}
	DEBUG("end\n");
}
// 清除紅外線數據
void EWC_IR::cleardata() {
	for (int i = 0; i < irMaxLen; i++) {
		irBuffer[i] = 0;
	}
	ir_x = 0;
	irStartRecv = false;
}

/********************************************************************************
   蜂鳴器或燈號指示
*/
// IR開始蜂鳴(燈號)
void EWC_IR::beeStart() {
	if (SIGN_PIN == 255) {
		return;
	}
	digitalWrite(SIGN_PIN, HIGH);
	delay(100);
	digitalWrite(SIGN_PIN, LOW);
}
// IR結束蜂鳴(燈號)
void EWC_IR::beeEnd() {
	if (SIGN_PIN == 255) {
		return;
	}
	for (int k = 0; k < 2; k++)  {
		digitalWrite(SIGN_PIN, HIGH);
		delay(100);
		digitalWrite(SIGN_PIN, LOW);
		delay(100);
	}
}

/********************************************************************************
   紅外線接收寫入使用 rt1.raw
*/
boolean EWC_IR::saveScanRaw(String item) {
  String file = INFO_PATH;
  file += item;
  file += ".raw";
  cleardata();
  beeStart();
  unsigned long previous_time, current_time, time_passed, out_passed;
  unsigned long learnTime = 800;  // 定義接收開始到結束最的時間(原int將資料型別與out_passed一致)
  out_passed = 6000;  // 定義等待學習最多6秒
  previous_time = millis();
  DEBUG_IR("IR RECV start..." + String(ir_x));
  delay(100);
  attachInterrupt(digitalPinToInterrupt(RECV_PIN), rxIR_Interrupt_Handler, CHANGE);  //set up ISR for receiving IR signal
  while (true) {
    if (irStartRecv) {  // 開始紅外線數據攔截
      if (out_passed != learnTime) {
        out_passed = learnTime;
        previous_time = millis(); //重新計算1秒接收
      }
    }
    current_time = millis();
    if (current_time >= previous_time )
      time_passed = current_time - previous_time;
    else
      time_passed = ULONG_MAX - previous_time + current_time;
    if (time_passed > out_passed)
      break;
	yield();  //重置看門狗(Feed the WDT)才不會崩潰
  }
  detachInterrupt(digitalPinToInterrupt(RECV_PIN));  // stop interrupts & capture until finshed here
  DEBUG_IR("IR RECV end..." + String(ir_x));
  beeEnd();
  if (out_passed == learnTime)
    DEBUG_IR("IR recv study OK");
  else {
    DEBUG_IR("IR recv study ERR");
    return false;
  }
  printRaw();  //顯示結果
  // 開始寫入raw檔
  DEBUG("save file is ");
  DEBUG(file+"\n");
  File rawFile = SPIFFS.open(file, "w");
  if (!rawFile) {
    DEBUG_IR("Failed to open raw file for writing");
    return false;
  }
  rawFile.print(String(ir_x - 1));
  rawFile.print(",");
  for (int i = 1; i < ir_x; i++) {
    rawFile.print(String(irBuffer[i] - irBuffer[i - 1]));
    rawFile.print(",");
  }
  rawFile.print("\r\n");
  rawFile.close();
  return true;
}

/********************************************************************************
   紅外線讀取,輸入 st1 轉 rt1.raw 讀取
*/
boolean EWC_IR::sendIrRaw(String item) {
  String file = INFO_PATH;
  file += item;
  file += ".raw";
  //file.replace("st", "rt");原始網頁專案轉換使用
  beeStart();
  DEBUG_IR(file);
  // 開始讀取raw檔
  File rawFile = SPIFFS.open(file, "r");
  if (!rawFile) {
    DEBUG_IR("Failed to open raw file for reading");
    return false;
  }
  cleardata();
  unsigned int ar_num = 0;
  char c;
  String com_char = "";
  int firstnum = 0;
  unsigned int irTmp;
  while (rawFile.available()) {
    c = rawFile.read();
    if (c != ',' ) {
      com_char += c;
    } else {
      irTmp = com_char.toInt();
      if (ar_num == 0)
        firstnum = irTmp;
      else
        irBuffer[ar_num - 1] = irTmp;
      DEBUG(String(irTmp));
      DEBUG(",");
      com_char = "";
      if (ar_num < irMaxLen)
        ar_num++;
    }
  }
  rawFile.close();
  DEBUG("end\n");
  // 發射紅外線訊號
  //irsend.sendRaw(const_cast<unsigned int*> (irBuffer), firstnum, 38);
  sendRaw(const_cast<unsigned int*> (irBuffer), firstnum, 38);
  delay(300);
  return true;
}

//*******************************************************************************
////// Private: //////
//*******************************************************************************

template <typename Generic>
void EWC_IR::DEBUG_IR(Generic text) {
	if (isDebug) {
		DEBUG_TAG();
		DEBUG(text);
		DEBUG("\n");
	}
}

template <typename Generic>
void EWC_IR::DEBUG(Generic text) {
	if (isDebug) {
		Serial.print(text);
	}
}

void EWC_IR::DEBUG_TAG() {
	if (isDebug) {
		Serial.print("*IR: ");
	}
}

/********************************************************************************
   紅外線接收中斷
*/
static void ICACHE_RAM_ATTR rxIR_Interrupt_Handler(void) {
  static unsigned long time0;
  uint16_t irb;
  unsigned long xtime;
  if (ir_x >= (irMaxLen)) {
    return;  // ignore if irBuffer is already full
  }
  if (ir_x == 0) {
    time0 = micros();
    irBuffer[ir_x] = 0;
    irStartRecv = true;
  } else {
    xtime = micros();
    if (xtime >= time0)
      irBuffer[ir_x] = xtime - time0;
    else
      irBuffer[ir_x] = ULONG_MAX - time0 + xtime;
  }
  ir_x++;
}

void EWC_IR::sendRaw(unsigned int buf[], int len, int hz)
{
  enableIROut(hz);
  for (int i = 0; i < len; i++) {
    if (i & 1) {
      space(buf[i]);
    }
    else {
      mark(buf[i]);
    }
  }
  space(0); // Just to be sure
}

void EWC_IR::mark(int time) {
  long beginning = micros();
  while (micros() - beginning < time) {
    digitalWrite(SEND_PIN, HIGH);
    delayMicroseconds(halfPeriodicTime);
    digitalWrite(SEND_PIN, LOW);
    delayMicroseconds(halfPeriodicTime); //38 kHz -> T = 26.31 microsec (periodic time), half of it is 13
  }
}

void EWC_IR::space(int time) {
  digitalWrite(SEND_PIN, LOW);
  if (time > 0) delayMicroseconds(time);
}

// 定義紅外線頻率
void EWC_IR::enableIROut(int khz) {
  // Enables IR output.  The khz value controls the modulation frequency in kilohertz.
  //halfPeriodicTime = 500 / khz; // T = 1/f but we need T/2 in microsecond and f is in kHz
  halfPeriodicTime = 13;
}


