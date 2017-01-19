/**************************************************/
/*  Smart PV Transceiver . Receiver Prototype     */
/*  create: 2016-10-07                            */
/*  update: 2016-10-07                            */
/**************************************************/

// ピンレイアウトの定義
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

#define PULSE_DRIVE_DURATION    200
#define PULSE_ZERO_DURATION     800
#define PULSE_ONE_DURATION     1600
#define PULSE_BREAK_DURATION   5000

int duration_counter=0;
int bit_index=0;
byte n_recv_packet=0;
byte preamble=0;
//ここの数字決めつけるべきではなさそう
byte recv_packet[5]; //ID-command-V_I-CRC ここまで準備しておけばひとまず十分？
byte send_packet[4]; //ID-commnad-CRC (自分のIDを入力しておく？) 返信・コマンドで長さ変わる...フラグ

byte device_id=0;
byte voltage=0;
byte temperature=0;

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

void createCommand(){
  //ここで目的ノードと命令を指定させる[0][1]
  /*for(int i =0;i<2;i++){
    (Serial.available() > 0) {
    send_packet[i] = Serial.read(); //id - command
    i ++;
    }
  }*/
  int dest_id = 1; //とりあえず決め打ち
  int command = 1; //とりあえずこれも決め打ち
  send_packet[0] = dest_id;
  send_packet[1] = command;
  send_packet[2] = device_id; //返信用の自分のアドレス
  unsigned short send_packet_crc=crc(send_packet,3);
  send_packet[3]= byte(send_packet_crc);
}

//とりあえずcommand =1(状態を知りたい)
void createResppacket(){
  int command = 10; //適当
  send_packet[0] = recv_packet[2]; //仮 実際には送信されてきたid
  send_packet[1] = command;
  send_packet[2] = voltage;
  send_packet[3] = temperature;
  unsigned short send_packet_crc=crc(send_packet,4);
  send_packet[4]= byte(send_packet_crc);
}

void getDeviceStatus(){        
  device_id=0;
  if(digitalRead(SW_BIT0)==LOW){ device_id+=1; }
  if(digitalRead(SW_BIT1)==LOW){ device_id+=2; }
  if(digitalRead(SW_BIT2)==LOW){ device_id+=4; }
  if(digitalRead(SW_BIT3)==LOW){ device_id+=8; }
  if(digitalRead(SW_BIT4)==LOW){ device_id+=16; }
  if(digitalRead(SW_BIT5)==LOW){ device_id+=32; }

  int volt=analogRead(VOLTAGE);
  voltage=(byte)((((long)volt*560UL/1024/12)*2+1)/2);  //  = AD*5/1024*(100+12)/12

  int temp=analogRead(TEMPERATURE);
  temperature=(byte)(temp*500/1024);
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

//Nの個数だけbyte送信 →ID-command-CRCで問題ない
//for loopの回数を調整 引数で渡せばいいか
void sendPacket(int N){
  int i,j;
  byte* p=send_packet;
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

void recvPacket(){
  int sig;
  int psig=analogRead(CT_SIGNAL);
  byte this_bit;
  n_recv_packet=0;

  while(1){
    //受信部分
    sig=analogRead(CT_SIGNAL);
    
    if(sig-psig>100){
      // 9 ==> 0;  16 ==> 1  (13 should be the threshold)
      //Serial.println(duration_counter);
      if(duration_counter<5){
        // Do Nothing (destroy garbage)
      }else if(duration_counter<13){
        //this_bit=0;
        if(++bit_index>=5){
          byte byte_index=(bit_index-5)>>3;
          recv_packet[byte_index]>>=1;
          Serial.print(0);
        }
      }else if(duration_counter<26){
      //this_bit=1;
        if(++bit_index>=5){
          byte byte_index=(bit_index-5)>>3;
          recv_packet[byte_index]>>=1;
          recv_packet[byte_index]|=0x80;
          Serial.print(1);
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
    if(++duration_counter>5000 && bit_index>0){ //ここまでok
      Serial.println("");
      digitalWrite(LED_RX,HIGH);
      if (recv_packet[0] == byte(device_id)) {  
        Serial.println(recv_packet[0]);
        Serial.println(recv_packet[1]);
        Serial.println(recv_packet[2]);
        Serial.println(recv_packet[3]);
        n_recv_packet=(bit_index-5)>>3;
        Serial.println(n_recv_packet);
        unsigned short recv_packet_crc=crc(recv_packet,n_recv_packet);
        if(recv_packet[n_recv_packet]==byte(recv_packet_crc)){
            
            if (recv_packet[1] == 1){ //決め打ちcommand受信
              //送信モードに入る必要あり
              getDeviceStatus();
              createResppacket();
              // infoは5byteなのでsnedPacketの引数5
              sendPacket(5);
              }else if (recv_packet[1] == 10){
                Serial.print("ID="); Serial.print(recv_packet[0]);
                Serial.print("; V="); Serial.print(recv_packet[1]);
                Serial.print("; T="); Serial.println(recv_packet[2]);
              }
            }
          }
      digitalWrite(LED_RX,LOW);
      bit_index=0;
      duration_counter=5000;
    }
    
    //command送信側となる
    if (Serial.available() > 0){
      Serial.read();
      getDeviceStatus();
      createCommand();
      // commandは4byteなのでsnedPacketの引数4
      sendPacket(4);
    }
  }
}

// 初期化プログラム
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

// mainルーチン
void loop(){
  getDeviceStatus();
  recvPacket();
}


