#include "PVcommunicate_breadboard.h"

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

/*
void PV::ltica(){
    digitalWrite(LED_RX,HIGH);
    delay(1000);
    digitalWrite(LED_RX,LOW);
    delay(1000);
}
*/

void PV::generatepacket(){
  createpacket();
  showpacket();
  sendPacket(getlpacket());
}

void PV::getstatus(){
  //fixed
  ID = 1;
  voltage = 5;
  temperature = 20;
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
  /*
  Vt = 0;
  Vt_ = 0;
  Vavg = 0;
  Vavg_ = 0;
  Vmax = 0;
  */
  error = 0;
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
  //lpacket は全てのパケットではないぞ
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
      //error 内容　今回はひとまず 0を送ります
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
  //lpacket = lpacket + dist_ID + contencts
  //CRC → made from all before packet
  // +2 means ID and command
  //all packet は lpacket +3がもっとも間違いがない
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
  //ここで+3送る仕様にしているので問題がない
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
  showpacket();
}

void PV::setcommand(byte command){
  //_command = DateRespになっていた
  _command = command ;
}
void PV::setdistID(byte _distID){
  //_command = DateRespになっていた
  dist_ID = _distID ;
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
  if (Serial.available() > 0){
    init();
    getstatus();
    decidecommand();
    createpacket();
    Serial.println("send command!");
    //showpacket();
    sendPacket(getlpacket());
  }
  //受信部分
  sig=analogRead(CT_SIGNAL);
  if(sig-psig >100){
    /*
    Serial.print("pulse:");
    Serial.println(duration_counter);
    Serial.print("sig-psig");
    Serial.println(sig);
    Serial.println(psig);
    */
    // 9 ==> 0;  16 ==> 1  (13 should be the threshold)
    if(duration_counter<20){
      // Do Nothing (destroy garbage)
    // 13 -> 10 10で分かれそうだった
   }else if(duration_counter<100){
      //this_bit=0;
      if(++bit_index>=5){
        //++が先に処理が来るから４ではなく5
        byte byte_index=(bit_index-5)>>3;
        packet[byte_index]>>=1;
      }
    }else if(duration_counter<200){
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
    showpacket();
    //bit_index >0でそもそもbitあるかどうか
    //+3はDistID,command,CRC
    //bitの数があっているか
    //broadcastに対応
    if (packet[0] == ID || packet[0] == 255) {
      n_recv_packet=(bit_index-4)>>3;
      Serial.println("ID is ok");
      if (n_recv_packet == packet[2] + 3) {
        Serial.println("packet size is ok");
        unsigned short recv_packet_crc=crc(packet,n_recv_packet -1 );
        if(packet[n_recv_packet-1]==byte(recv_packet_crc)){
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
              break;
            case ack :
              Serial.println("Command was send correctly");
              break;
            case disconnect :
              //順番としては　受信-ack送信-切断
              //ack受診までまてばより確実だが．．．．
              setcommand(ack);
              dist_ID = packet[3];
              generatepacket();
              Serial.println("disconnect");
            //normally disconnect
              digitalWrite(relay,HIGH);
            //normally connect
              digitalWrite(harvest,HIGH);
              break;
            case recovery :
              digitalWrite(relay,LOW);
              digitalWrite(harvest,LOW);
            //これくらい待てば復帰できる?
              delay(1600);
              setcommand(ack);
              dist_ID = packet[3];
              generatepacket();
              Serial.println("recovery");
              break;
            }
          }else{
            Serial.println("CRC not match");
            setcommand(Error);
            error = 1;
            //error内容
            dist_ID = packet[3];
            generatepacket();
          }
        //次はbit数がおかしい
      }else{
        Serial.println("number of bit is not good");
        setcommand(Error);
        dist_ID = packet[3];
        error = 2;
        //error内容
        generatepacket();
      }
    } // IDが違う 特に何もしなくて良さそう
    //digitalWrite(LED_RX,LOW);
    init();
    //読み取り終了でinit実行
  }
}
