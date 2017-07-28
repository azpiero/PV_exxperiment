#include <wiring_private.h>
#include <PV_comdis.h>
#include <MsTimer2.h>
//#include <EEPROM.h>

PV pv;

void broadcast(){
  Serial.println("send broadcast");
  pv.setcommand(DataReq);
  pv.setdistID(255);
  pv.generatepacket();
}

void setup() {
 Serial.begin (115200);
 Serial.println("Starting...");
 pinMode(L_DRIVE,OUTPUT);
 pinMode(relay,OUTPUT);
 pinMode(harvest,OUTPUT);
 //常にON
 digitalWrite(harvest,HIGH);
 pinMode(LED_TX,OUTPUT);
 pinMode(LED_RX,OUTPUT);
 pinMode(SW_BIT0,INPUT_PULLUP);
 pinMode(SW_BIT1,INPUT_PULLUP);
 pinMode(SW_BIT2,INPUT_PULLUP);
 pinMode(SW_BIT3,INPUT_PULLUP);
 pinMode(SW_BIT4,INPUT_PULLUP);
 pinMode(SW_BIT5,INPUT_PULLUP);
 sbi(ADCSRA, ADPS2); //もともと cbi  adps0 prescale (set cbi 0 sbi 1)
 cbi(ADCSRA, ADPS1); //         sbi
 cbi(ADCSRA, ADPS0); //         cbi
 pv.getstatus();
 pv.showstatus();
 Serial.println("please enter dist ID and command");
 Serial.println("command 1-datareq 3-communicate");
  //30000UL-10000にしとく
  MsTimer2::set(10000UL, broadcast);
  MsTimer2::start();
  /*
  for(int a = 0;a<256;a++){
    EEPROM.write(a,a);
  }
  */
}

void loop() {
  pv.resvPacket();
}
