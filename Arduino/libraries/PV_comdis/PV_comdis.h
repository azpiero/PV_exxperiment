#ifndef PV_comdis_h
#define PV_comdis_h


#if defined(ARDUINO) && ARDUINO >= 100
	#include "Arduino.h"
#else
	#include "WProgram.h"
#endif

#include "MsTimer2.h"
#include "TimerOne.h"
//#include "EEPROM.h"

#define CT_SIGNAL A0
#define VOLTAGE A1
#define TEMPERATURE A2
#define LED_TX A3
#define LED_RX A4

#define L_DRIVE 2
//harvestは常にhighで！(Normally open)
#define harvest 3
#define relay 	4
#define SW_BIT0 5
#define SW_BIT1 6
#define SW_BIT2 7
#define SW_BIT3 8
#define SW_BIT4 9
#define SW_BIT5 10

#define DataReq 		1
#define check				2
#define communicate 3
#define recovery 		4
#define DataResp 		5
#define ack 				6
#define Error 			7
#define resend			8
#define disconnect	9

#define PULSE_DRIVE_DURATION    200
#define PULSE_ZERO_DURATION     800
#define PULSE_ONE_DURATION     1600
#define PULSE_BREAK_DURATION   5000

class PV{
public:
  void ltica();
  void getstatus();
  void decidecommand();
  void setcommand(byte command);
  void sendPacket(int length);
  void resvPacket();
  void showstatus();
  void createpacket();
  void init();
  int getlpacket();
  void showpacket();
  void generatepacket();
  void setdistID(byte dist_ID);
  bool crccheck();
  bool sizecheck();
  int waitID();

  //Serialからの命令読み取り用
	bool decidecommandflag = false;
	int decidecommnadcounter = 0;

  //slave用timerone
	bool timeroneflag = false;
	int loopcounter = 0;

private:
  byte ID;
  byte dist_ID;
  byte voltage;
  byte temperature;
  byte _command;
  byte error;
  byte packet[20];
  byte sendpacket[20];
  int lpacket = 2;
  int bit_index = 0;
  int sig=0;
  int psig=0;
  byte this_bit=0;
  byte n_recv_packet=0;
  int duration_counter=0;

	//byte EEPROM_buffer[256];
	//int EEPROM_counter = -1;

};

#endif
