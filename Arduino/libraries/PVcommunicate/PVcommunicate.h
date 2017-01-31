#ifndef PVcommunicate_h
#define PVcommunicate_h


#if defined(ARDUINO) && ARDUINO >= 100
	#include "Arduino.h"
#else
	#include "WProgram.h"
#endif

#define CT_SIGNAL A0
#define VOLTAGE A1
#define TEMPERATURE A2
#define LED_TX A3
#define LED_RX A4
#define L_DRIVE 2
#define SW_BIT0 4
#define SW_BIT1 5
#define SW_BIT2 6
#define SW_BIT3 7
#define SW_BIT4 8
#define SW_BIT5 9
#define SW_TRANSMIT 11

//command list こういう書き方でいいのか
#define DataReq 		10
#define syn 				11
#define communicate 12
#define Ltica 			13
#define DataResp 		20
#define ack 				21
#define Error 			30


#define PULSE_DRIVE_DURATION    200
#define PULSE_ZERO_DURATION     800
#define PULSE_ONE_DURATION     1600
#define PULSE_BREAK_DURATION   5000

//決め打ちよりはn++して最後にその長さを見る方が賢い気がする これだと固定長

class PV{
public:
  void ltica();
  void getstatus();
	void decidecommand();
	void setcommand(byte command);
  void sendPacket(int length);
  void resvPacket();
	//debug
  void showstatus();
	void createpacket();
	void init();
	int getlpacket();

private:
  byte ID;
  byte dist_ID;
  byte voltage;
  byte temperature;
	byte _command;
	// std::vector<byte> send_packet;
	// std::vector<byte> resp_packet;
	//可変長にしたいけどひとまず保留
	byte packet[20];
	int lpacket = 2;
	int bit_index;
};

#endif
