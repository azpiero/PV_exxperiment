#include <PVcommunicate.h>

PV pv;

void setup(){
  Serial.begin(115200);
  Serial.println("Starting...");
  pinMode(L_DRIVE,OUTPUT);
  pinMode(LED_TX,OUTPUT);
  pinMode(LED_RX,OUTPUT);
  pinMode(SW_BIT0,INPUT_PULLUP);
  pinMode(SW_BIT1,INPUT_PULLUP);
  pinMode(SW_BIT2,INPUT_PULLUP);
  pinMode(SW_BIT3,INPUT_PULLUP);
  pinMode(SW_BIT4,INPUT_PULLUP);
  pinMode(SW_BIT5,INPUT_PULLUP);
  pinMode(SW_TRANSMIT,INPUT_PULLUP);
}

void loop(){
  //pv.ltica();
  pv.resvPacket();

  /*pv.getstatus();
  pv.decidecommand();
  pv.createpacket();
  pv.showstatus();
  pv.sendpacket(pv.getlpacket());*/
  //delay(1000);

}
