#include "PVcommunicate.h"

//おまじない
#if defined(ARDUINO) && ARDUINO >= 100
#include "Arduino.h"
#else
#include "WProgram.h"
#endif


unsigned short crc( unsigned const char *pData, unsigned long lNum )
{
  unsigned int crc16 = 0xFFFFU;
  unsigned long i;
  int j;

  for ( i = 0 ; i < lNum ; i++ ){
    crc16 ^= (unsigned int)pData[i];
    for ( j = 0 ; j < 8 ; j++ ){
      if ( crc16 & 0x0001 ){
        crc16 = (crc16 >> 1) ^ 0xA001;
      }else{
        crc16 >>= 1;
      }
    }
  }
  return (unsigned short)(crc16);
}

void PV::ltica(){
    digitalWrite(LED_RX,HIGH);
    delay(1000);
    digitalWrite(LED_RX,LOW);
    delay(1000);
}


void PV::getstatus(){
    byte device_id = 0;
    if(digitalRead(SW_BIT0)==LOW){ device_id+=1; }
    if(digitalRead(SW_BIT1)==LOW){ device_id+=2; }
    if(digitalRead(SW_BIT2)==LOW){ device_id+=4; }
    if(digitalRead(SW_BIT3)==LOW){ device_id+=8; }
    if(digitalRead(SW_BIT4)==LOW){ device_id+=16; }
    if(digitalRead(SW_BIT5)==LOW){ device_id+=32; }

    ID = device_id;

    int volt=analogRead(VOLTAGE);
    voltage=(byte)((((long)volt*560UL/1024/12)*2+1)/2);  //  = AD*5/1024*(100+12)/12
    int temp=analogRead(TEMPERATURE);
    temperature=(byte)(temp*500/1024);

}

void PV::showstatus(){
    // Serial.print(ID);
    // Serial.print(voltage);
    // Serial.println(temperature);
    for(int t=0;t<lpacket+3;t++){
      Serial.print(packet[t]);
      Serial.print(" ");
    }
    Serial.println("");

    for(int i = 0;i<10;i++){
      packet[i] = 0;
    }
}

void PV::decidecommand(){
  Serial.println("please enter dist ID and command");
  while(1){
    if (Serial.available() > 0) {
      dist_ID = (byte)(Serial.read() - '0');
      break;
    }
  }
  while(1){
    if (Serial.available() > 0){
      _command = (byte)(Serial.read() - '0');
      break;
    }
  }
}


void PV::init(){
  lpacket = 2;
}

int PV::getlpacket(){
  return lpacket;
}

void PV::createpacket(){
  int i = 4;
  int t = 0;
  init();
  packet[0] = dist_ID;
  packet[1] = _command;
  //lpacket必要な場面少なそう
  packet[3] = ID;
  switch(_command){
  //case 10 :
  //case 11 :
    case 1 :
      Serial.println("please enter message");
      while(t<10){
        if(Serial.available()>0){
          packet[i] = (byte)(Serial.read());
          i++;
          lpacket ++;
        }
        delay(1000);
        t++;
      }
      break;
    //case 13 :
    case 2 :
      getstatus();
      packet[4] = voltage;
      packet[5] = temperature;
      lpacket = 4;
      break;
      //case 21 :
        //no error 0 error 1
      //case 30 :
  }
  packet[2] = (byte)lpacket;
  packet[lpacket+2]=byte(crc(packet,lpacket+2));

  //lpacket + 2(command +dist_ID ) is all packet length
}

void sendZero(){
  digitalWrite(L_DRIVE,HIGH);
  delayMicroseconds(PULSE_DRIVE_DURATION);
  digitalWrite(L_DRIVE,LOW);
  delayMicroseconds(PULSE_ZERO_DURATION);
}

void sendOne(){
  digitalWrite(L_DRIVE,HIGH);
  delayMicroseconds(PULSE_DRIVE_DURATION);
  digitalWrite(L_DRIVE,LOW);
  delayMicroseconds(PULSE_ONE_DURATION);
}

void sendBreak(){
  digitalWrite(L_DRIVE,HIGH);
  delayMicroseconds(PULSE_DRIVE_DURATION);
  digitalWrite(L_DRIVE,LOW);
  delayMicroseconds(PULSE_BREAK_DURATION);
}

void sendPreamble(){
  sendOne();   sendOne();   sendOne();   sendOne();
}

void sendPostamble(){
  sendBreak();
}

//for loopの回数を調整 引数で渡せばいいか
void PV::sendPacket(int N){
  int i,j;
  byte* p=packet;
  sendPreamble();
  for(i=0;i<N;i++,p++){
    for(j=1;j<256;j<<=1){
      if(*p&j){
        sendOne();
        Serial.print(1);
      }else{
        sendZero();
        Serial.print(0);
      }
    }
  }
  Serial.println("Send!!");
  sendPostamble();
}


void PV::setcommand(byte command){
  switch (command){
    case 10:
      _command = 20 ;
      break;
    }
}

void PV::resvPacket(){
  int sig;
  int psig=analogRead(CT_SIGNAL);
  byte this_bit;
  byte n_recv_packet=0;
  int duration_counter=0;

  while(1){
    //command送信側となる
    if (Serial.available() > 0){
      getstatus();
      decidecommand();
      createpacket();
      showstatus();
      sendPacket(getlpacket());
    }
    //受信部分
    sig=analogRead(CT_SIGNAL);
    if(sig-psig>150){
      // 9 ==> 0;  16 ==> 1  (13 should be the threshold)
      if(duration_counter<5){
        // Do Nothing (destroy garbage)
      }else if(duration_counter<13){
        //this_bit=0;
        if(++bit_index>=5){
          byte byte_index=(bit_index-5)>>3;
          //Serial.println(byte_index);
          packet[byte_index]>>=1;
        }
      }else if(duration_counter<26){
        //this_bit=1;
        if(++bit_index>=5){
          byte byte_index=(bit_index-5)>>3;
          //Serial.println(byte_index);
          packet[byte_index]>>=1;
          packet[byte_index]|=0x80;
        }
      }else{
        //  this_bit=2;   // start -- detected
        bit_index=0;
        n_recv_packet=0;
      }
      duration_counter=0;
    }
    psig=sig;

    //解読部分
    if(++duration_counter>5000 || bit_index>=255){
      // break detected
      if(bit_index>0){
        digitalWrite(LED_RX,HIGH);
        if (packet[0] == ID) {
          n_recv_packet=(bit_index-5)>>3;
          unsigned short recv_packet_crc=crc(packet,n_recv_packet -1 );
          if(packet[4]==byte(recv_packet_crc)){
            if (packet[1] == 10){ //決め打ちcommand受信
              //送信モードに入る必要あり
              getstatus();
              //受信した命令に対してcommnadを設定
              setcommand(packet[1]);
              createpacket();
              // infoは5byteなのでsnedPacketの引数5
              sendPacket(5);
            }else if (packet[1] == 20){
              Serial.print("ID="); Serial.print(packet[0]);
              Serial.print("; V="); Serial.print(packet[1]);
              Serial.print("; T="); Serial.println(packet[2]);
            }
          }
        }
        digitalWrite(LED_RX,LOW);
        bit_index=0;
        break;
      }
      duration_counter=5000;
    }
  }
}
