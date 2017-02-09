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
#define DataReq 		1
#define syn 				2
#define communicate 3
#define Ltica 			4
#define DataResp 		5
#define ack 				6
#define Error 			7
#define resend			8


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
	void showpacket();
	//確認用

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
	int bit_index = 0;
	//コメント表示用
	int sig=0;
	int psig=0;
	byte this_bit=0;
	byte n_recv_packet=0;
	int duration_counter=0;
};

#endif
