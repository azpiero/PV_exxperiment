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
  Serial.print(ID);
  Serial.print(" ");
  Serial.print(voltage);
  Serial.print(" ");
  Serial.println(temperature);
}

void PV::decidecommand(){
  while(1){
    if (Serial.available() > 0) {
      dist_ID = (byte)(Serial.read() - '0');
      Serial.print("dist ID :");
      Serial.print(dist_ID);
      Serial.print(" ");
      break;
    }
  }
  while(1){
    if (Serial.available() > 0){
      _command = (byte)(Serial.read() - '0');
      Serial.print("commnad :");
      Serial.println(_command);
      break;
    }
  }
  Serial.read();
  //改行文字
}

void PV::init(){
  lpacket = 2;
  bit_index =0;
  sig=0;
  psig=0;
  this_bit=0;
  n_recv_packet=0;
  duration_counter=5000;
  for(int i =0;i<20;i++){
    packet[i] = 0;
    sendpacket[i]=0;
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
      sendpacket[4] = 0;
      lpacket = 3;
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

//for loopの回数を調整 引数で渡せばいいか
void PV::sendPacket(int N){
  //試し
  //unsigned long time;
  //unsigned long ptime;
  int i,j;
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
      //Serial.println(time - ptime);
      //ptime = time;
    }
  }
  sendPostamble();
}

void PV::setcommand(byte command){
  _command = DataResp ;
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

void PV::resvPacket(){
  bool flag = 0;
  //送信者が受信者か(送信者が1になる)
  //command送信側となる
  if (Serial.available() > 0){
    init();
    getstatus();
    decidecommand();
    createpacket();
    Serial.print("send command!");
    //showpacket();
    sendPacket(getlpacket());
    flag = 1;
  }
  //受信部分
  sig=analogRead(CT_SIGNAL);
  //このスレッショルドを低くすると見逃しへる　but 見間違えも増える？？
  if(sig-psig>80){
    // 9 ==> 0;  16 ==> 1  (13 should be the threshold)
    if(duration_counter<5){
      // Do Nothing (destroy garbage)
    }else if(duration_counter<13){
      //this_bit=0;
      if(++bit_index>=5){
        byte byte_index=(bit_index-5)>>3;
        packet[byte_index]>>=1;
      }
    }else if(duration_counter<26){
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
  if(++duration_counter>5000 && bit_index>0){
    //showpacket();
    // break detected
    digitalWrite(LED_RX,HIGH);
    if (packet[0] == ID) {
      n_recv_packet=(bit_index-4)>>3;
      unsigned short recv_packet_crc=crc(packet,n_recv_packet -1 );
      if(packet[n_recv_packet-1]==byte(recv_packet_crc)){
        if (packet[1] == DataReq){ //決め打ちcommand受信
          Serial.println("recv DataReq!");
          //送信モードに入る必要あり
          delay(1000);
          getstatus();
          dist_ID = packet[3]; //distID command length ID
          //受信した命令に対してcommnadを設定
          setcommand(DataResp);
          createpacket();
          // infoは5byteなのでsnedPacketの引数5
          Serial.print("send response!");
          //showpacket();
          sendPacket(getlpacket());
        }else if (packet[1] == DataResp){
          Serial.println("recv DataResp!");
          Serial.print("ID="); Serial.print(packet[3]);
          Serial.print("; V="); Serial.print(packet[4]);
          Serial.print("; T="); Serial.println(packet[5]);
        }else if (packet[1] == communicate){ //決め打ちcommand受信
          Serial.println("recv Communicate!");
          //中身を文字として表示したい
          //content の長さは 4からpacket length +2まで 2に注意
          for(int i =4;i<packet[2]+2;i++){
            Serial.print(char(packet[i]));
          }
        }else if(packet[1] == Error){
          sendPacket(getlpacket());
        }
      }else{
        //受信サイド error返信
        if(flag == 0){
          Serial.println("receive answer incorrectly");
          dist_ID = packet[3];
          setcommand(Error);
          createpacket();
          Serial.print("send ");
          //showpacket();
          sendPacket(getlpacket());
          //error報告
        }else if(flag == 1){
          //送信サイド
          Serial.println(flag);
          Serial.println("receive answer incorrectly");
          //同じものをそうしん
          sendPacket(getlpacket());
        }
        //とりあえず再送要求
        //送信したパケットが消えていて欲しくないかも
      }
    }
    digitalWrite(LED_RX,LOW);
    init();
    flag = 0;
  }
}
