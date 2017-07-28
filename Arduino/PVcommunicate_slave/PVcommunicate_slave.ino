#include <wiring_private.h>
#include <PV_comdis.h>
#include <MsTimer2.h>
#include <TimerOne.h>
//#include <EEPROM.h>

PV pv;

void respheartbeat(){
  pv.generatepacket();
  Serial.println("resp heartbeat!");
  MsTimer2::stop();
}

void errorDisconnect(){
  if(++pv.loopcounter >60){
  //Serial.println("disconnect by no heartbeat");
    digitalWrite(harvest,LOW);
    digitalWrite(relay,HIGH);
    digitalWrite(LED_RX,HIGH);
    digitalWrite(LED_TX,HIGH);
    pv.loopcounter = 0;
  }
}

void setup(){
  Serial.begin(115200);
  Serial.println("Starting...");
  pinMode(L_DRIVE,OUTPUT);
  pinMode(relay,OUTPUT);
  pinMode(harvest,OUTPUT);
  //つねにON
  digitalWrite(harvest,HIGH);
  pinMode(LED_TX,OUTPUT);
  pinMode(LED_RX,OUTPUT);
  pinMode(SW_BIT0,INPUT_PULLUP);
  pinMode(SW_BIT1,INPUT_PULLUP);
  pinMode(SW_BIT2,INPUT_PULLUP);
  pinMode(SW_BIT3,INPUT_PULLUP);
  pinMode(SW_BIT4,INPUT_PULLUP);
  pinMode(SW_BIT5,INPUT_PULLUP);
  sbi(ADCSRA, ADPS2);
  cbi(ADCSRA, ADPS1);
  cbi(ADCSRA, ADPS0);
  pv.getstatus();
  pv.showstatus();
  Serial.println("please enter dist ID and command");
  Serial.println("command 1-datareq 3-communicate");

  //heartbeatがなかった場合にerrordisconectを送信
  //timer1.initializeがheartbeart受信時に上書きされるか？
  Timer1.initialize(1000000L);
  Timer1.attachInterrupt(errorDisconnect);

  //heartbeat受信後一定時間待って返答
  //heartbeat受信時にrecoveryは行う
  MsTimer2::set(pv.waitID(),respheartbeat);
  MsTimer2::stop();

  //delay(1000);
  /*
  for(int a = 0;a<256;a++){
    EEPROM.write(a,a);
  }
  */

  while(Serial.available()>0){
    Serial.println(Serial.available());
    Serial.read();
  }
}

void loop(){
  pv.resvPacket();
}
