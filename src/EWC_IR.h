#ifndef __EWC_IR_h
#define __EWC_IR_h

#include <ESP8266WiFi.h>

	//自訂紅外線收發
static const int irMaxLen = 1024; //紅外線最大接收長度
static volatile unsigned int irBuffer[irMaxLen]; //stores timings - volatile because changed by ISR
static volatile unsigned int ir_x = 0; //Pointer thru irBuffer - volatile because changed by ISR
static volatile boolean irStartRecv = false; //是否開始接收到紅外線訊號
static void ICACHE_RAM_ATTR rxIR_Interrupt_Handler(void);
	
class EWC_IR {
	

private:
	const byte UNDEFPIN = 255;  // 未設定 PIN 腳的預設值
	//const unsigned long ULONG_MAX = 4294967295UL; // 無號長整數最大值(ESP8266WiFi已包含此變數)

	byte RECV_PIN = UNDEFPIN;   // 接收 an IR detector
	byte SEND_PIN = UNDEFPIN;   // 發射 an IR led
	byte SIGN_PIN = UNDEFPIN;   // 訊號指示PIN腳
	
	String INFO_PATH = "/";
	int halfPeriodicTime;       //紅外線頻率
	
	bool isDebug = false;       // 除錯輸出
	
	template <typename Generic>
	void DEBUG_IR(Generic text);
	template <typename Generic>
	void DEBUG(Generic text);
	void DEBUG_TAG(void);
	
	void sendRaw(unsigned int buf[], int len, int hz);
	void mark(int time);
	void space(int time);
	void enableIROut(int khz);
	
	
public:
	EWC_IR(byte rpin, byte spin);
	EWC_IR(int rpin, int spin);
	EWC_IR(byte rpin, byte spin, byte beePin);
	EWC_IR(int rpin, int spin, int beePin);
	~EWC_IR(void);
	bool begin(void);
	void end(void);
	void setSign(byte beePIN);
	void setSign(int beePIN);
	void setDebug(bool isdbg);
	void setFilePath(String path);
	void printRaw(void);
	void cleardata(void);
	void beeStart();
	void beeEnd();
	boolean saveScanRaw(String item);
	boolean sendIrRaw(String item);

	
};

#endif
