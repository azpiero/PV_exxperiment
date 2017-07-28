#include "PV_comdis.h"

//おまじない
#if defined(ARDUINO) && ARDUINO >= 100
#include "Arduino.h"
#else
#include "WProgram.h"
#endif

void PV::generatepacket(){
  createpacket();
  showpacket();
  sendPacket(getlpacket());
}

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
    //256or512段階
  voltage=(byte)((((long)volt*560UL/1024/12)*2+1)/2);  //  = AD*5/1024*(100+12)/12
  int temp=analogRead(TEMPERATURE);
  temperature=(byte)(temp*500/1024);
}

void PV::showstatus(){
  Serial.print(ID);
  Serial.print(" ");
  Serial.print(voltage);
  Serial.print(" ");
  Serial.println(temperature);
}

void PV::decidecommand(){
  char buff;
  buff = (byte)(Serial.read());

  if(decidecommandflag){
    switch(decidecommnadcounter){
      case 0:
        dist_ID = buff - '0';
        Serial.print("dist ID :");
        Serial.print(dist_ID);
        Serial.print(" ");
        decidecommnadcounter++;
        break;
      case 1:
        _command = buff - '0';
        Serial.print("commnad :");
        Serial.println(_command);
        decidecommnadcounter++;
        break;
      }
    }

  switch (buff) {
    case 's' :
    Serial.println("stop heartbeat");
    MsTimer2::stop();
    case 'r' :
    Serial.println("restart heartbeat");
    MsTimer2::start();
    case '\n' :
    decidecommandflag = false;
    decidecommnadcounter = 0;
    break;
    case 'A' :
    decidecommandflag = true;
    //これで14とかのcommandを受け取れる
    break;
  }
}

void PV::init(){
  lpacket = 2;
  bit_index =0;
  error=0;
  sig=0;
  psig=0;
  this_bit=0;
  n_recv_packet=0;
  duration_counter=5000;
  for(int i =0;i<20;i++){
    packet[i] = 0;
    //sendpacket[i]=0;
    //受信したパケットはここでは初期化しない
  }
}

int PV::getlpacket(){
  return lpacket;
}

void PV::createpacket(){
  int i = 4;
  sendpacket[0] = dist_ID;
  sendpacket[1] = _command;
  //lpacket必要な場面少なそう
  sendpacket[3] = ID;
  switch(_command){
    case DataReq :
      Serial.println("request dist PV status");
      break;
    case communicate :
      Serial.println("please enter message");
      while(1){
        if(Serial.available()>0){
          char buff ;
          if((buff = (byte)(Serial.read())) == '\n'){
            break;
          }else{
            sendpacket[i] = buff;
            i++;
            lpacket ++;
          }
        }
      }
      break;
    case DataResp :
      getstatus();
      sendpacket[4] = voltage;
      sendpacket[5] = temperature;
      lpacket = 4;
      break;
    case Error :
      //error 内容
      sendpacket[4] = error;
      lpacket = 3;
      break;
    case disconnect :
      //内容は不要なはず
      lpacket = 2;
      break;
    case recovery:
      lpacket = 2;
      break;
    case ack :
      lpacket = 2;
      break;
    }
  sendpacket[2] = (byte)lpacket;
  sendpacket[lpacket+2]=byte(crc(sendpacket,lpacket+2));
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

void PV::sendPacket(int N){
  int i,j;
  //送信すればLEDが光る
  digitalWrite(LED_TX,HIGH);
  byte* p=sendpacket;
  sendPreamble();
  for(i=0;i<N+3;i++,p++){
    for(j=1;j<256;j<<=1){
      //time = micros();
      if(*p&j){
        sendOne();
      }else{
        sendZero();
      }
    }
  }
  sendPostamble();

  //再送はできなくなるけどsendpacket初期化の位置をここにしておく
  for(int i =0;i<20;i++){
    sendpacket[i] = 0;
  }
  digitalWrite(LED_TX,LOW);
}

void PV::setcommand(byte command){
  _command = command ;
}

void PV::setdistID(byte _distID){
  dist_ID = _distID ;
}

bool PV::sizecheck(){
  n_recv_packet=(bit_index-4)>>3;
  if ((n_recv_packet) == packet[2] + 3) {
    Serial.println("packet size is ok");
    return true;
  }else{
    Serial.println("packet size is NG");
    setcommand(Error);
    error= 2;
    //error内容
    generatepacket();
    return false;
  }
}

bool PV::crccheck(){
  unsigned short recv_packet_crc=crc(packet,n_recv_packet -1 );
  if(packet[n_recv_packet-1]==byte(recv_packet_crc)){
    Serial.println("CRC is ok");
    return true;
  }else{
    Serial.println("CRC is NG");
    setcommand(Error);
    error= 1;
    //error内容
    generatepacket();
    return false;
  }
}

void PV::showpacket(){
  for(int i =0;i<20;i++){
    Serial.print(sendpacket[i]);
    Serial.print(" ");
  }
  for(int i =0;i<20;i++){
    Serial.print(packet[i]);
    Serial.print(" ");
  }
  Serial.println("");
}

//slave用のmasterからのheartbeatに対するwait関数
int PV::waitID(){
  Serial.println(int(ID)*1000);
  return int(ID)*1000;
}

void PV::resvPacket(){

  if (Serial.available() > 0){
    //init();
    getstatus();
    while(Serial.available() > 0){
      decidecommand();
    }
    Serial.print("create command!");
    generatepacket();
    //showpacket();
  }

  //受信部分
  sig=analogRead(CT_SIGNAL);
  //EEPROMで確認用
  /*
  if(EEPROM_counter >=0 && EEPROM_counter <256){
    EEPROM_buffer[EEPROM_counter] = sig/4;
    EEPROM_counter ++;
    if(EEPROM_counter == 256){
      for(int a=0;a<256;++a){
        EEPROM.write(a,EEPROM_buffer[a]);
        }
    }
 }
 */
  //Serial.println(sig);
  if(sig >= 600 && psig < 600){
    //if(EEPROM_counter == -1) EEPROM_counter =0;
    //Serial.println(duration_counter);
    if(duration_counter<30){
      // Do Nothing (destroy garbage)
    }else if(duration_counter<60){
      //this_bit=0;
      if(++bit_index>=5){
        byte byte_index=(bit_index-5)>>3;
        packet[byte_index]>>=1;
      }
    }else if(duration_counter<120){
      //this_bit=1;
      if(++bit_index>=5){
        byte byte_index=(bit_index-5)>>3;
        packet[byte_index]>>=1;
        packet[byte_index]|=0x80;
      }
    }else{
      //ここ入るときはパルス見逃している
      //  this_bit=2;   // start -- detected
      //bit_index=0;
    }
    duration_counter=0;
  }
  psig=sig;

  //解読部分
  if(++duration_counter>5000 && bit_index>4){
    //ちゃんとパケットを確認したらLEDを光らせる
    digitalWrite(LED_RX,HIGH);
    showpacket();
    Serial.print("pulse detect!!");
    if (packet[0] == ID) {
      if(sizecheck()){
        if(crccheck()){
          switch(packet[1]){
            case DataReq :
              Serial.println("recv DataReq!");
              delay(1000);
              getstatus();
              dist_ID = packet[3];
              setcommand(DataResp);
              generatepacket();
              Serial.println("send response!");
              break;
            case DataResp :
              Serial.println("recv DataResp!");
              Serial.print("ID="); Serial.print(packet[3]);
              Serial.print("; V="); Serial.print(packet[4]);
              Serial.print("; T="); Serial.println(packet[5]);
              break;
            case communicate : //決め打ちcommand受信
              Serial.println("recv Communicate!");
            //中身を文字として表示したい
            //content の長さは 4からpacket length +2まで 2に注意
              for(int i =4;i<packet[2]+2;i++){
                Serial.print(char(packet[i]));
              }
              break;
            case check :
              break;
            case Error :
              //delay(1000);
              //errorで切り分け id/command/length/ownID/contents
              switch(packet[4]){
                case 1 :
                  Serial.println("ERROR:CRC not match");
                  break;
                case 2 :
                  Serial.println("ERROR:packet size is invalid");
                  break;
              }
              /*
              Serial.println("send message again");
              showpacket();
              sendPacket(getlpacket());
              */
            case ack :
              Serial.println("Command was send correctly");
              break;
            case disconnect :
              setcommand(ack);
              dist_ID = packet[3];
              generatepacket();
              Serial.println("disconnect");
              //normally disconnect
              digitalWrite(relay,HIGH);
              //normally OPEN
              //if NC , LOW -> HIGH
              digitalWrite(harvest,LOW);
            break;
              break;
            case recovery :
              digitalWrite(relay,LOW);
              //normally OPEN
              //if NC , HIGH -> LOW
              digitalWrite(harvest,HIGH);
              delay(1600);
              setcommand(ack);
              dist_ID = packet[3];
              generatepacket();
              Serial.println("recovery");
                break;
              }
            }
          }
      //this is heartbeat
      }else if(packet[0] == 255){
        if(sizecheck()){
          if(crccheck()){
            switch(packet[1]){
              case DataReq :
                //切断復帰
                //カチッと毎回なって欲しくないのでもしそうなるならif(digitalreadできないのでどうするかは不明w)
                //bool relay で!で切り分けかなぁ
                digitalWrite(relay,LOW);
                digitalWrite(harvest,HIGH);
                //Timerone上書き　できる？？
                loopcounter =0;

                dist_ID = packet[3];
                setcommand(DataResp);
                Serial.println("recv heartbeat!");
                //slave特有の条件
                MsTimer2::start();
                break;
              }
            }
          }
        }
    init();
    digitalWrite(LED_RX,LOW);
  }
}
