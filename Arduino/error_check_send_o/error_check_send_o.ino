/**************************************************/
/*  Smart PV Transceiver . Transmitter Prototype  */
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
#define N 2

// デバイスの状態
byte device_id=0;
byte voltage=0;
byte temperature=0;

// デバイスの状態の取得
void getDeviceStatus(){        device_id=0;
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

byte send_packet[N]; 

#define PULSE_DRIVE_DURATION    200
#define PULSE_ZERO_DURATION     800
#define PULSE_ONE_DURATION     1600
#define PULSE_BREAK_DURATION   2400

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

void sendPacket(){
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

// 初期化プログラム
void setup(){
  Serial.begin(9600);

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
  for(int i = 0;i<N-1;i++){
    //send_packet[i]=random(-128,127);
    send_packet[i] = 0;
  }
  unsigned short send_packet_crc=crc(send_packet,N-1);
  send_packet[N-1]=byte(send_packet_crc);
  
  digitalWrite(LED_TX,HIGH);
  sendPacket();
  delay(1000);
  digitalWrite(LED_TX,LOW);
  /*
  for(int i=0;i<5;i++){
    delay(1000);
  }
  */
}

// CRC16の計算アルゴリズム
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

