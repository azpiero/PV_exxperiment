//送信側

#include <wiring_private.h> //analogread高速化
#define digitalPIN 9              // LEDはピン13に接続

int ON_reader = 200; // 400 2000 / 400 2400 /400 16000
int OFF_reader = 800;
int ON_1 = 200;
int OFF_1 = 1000;
int ON_0 = 200;
int OFF_0 = 600; //お試し

char Data[10]; //とりあえず10文字で
int pData = 0;

int flag = 1; //最初に到達したSerialか否か いらなそう

void setup() {
  Serial.begin(9600);
  pinMode(digitalPIN, OUTPUT);
}

void loop() {
  if (Serial.available() > 0) { //Serial.available()の返り値がデータのバイト数 charなら1byte 64までいける
    Data[pData] = Serial.read(); //c
    if (flag == 1) { //最初に必ずstartpulse活かせる data[0]はかならず最初のところだからいい
      send_startpulse(3);
      flag = 0;
    } else {
      send_message(0); //２文字目以降に必要
    }
  } else {
    flag = 1;
  }
}

void send_startpulse(int n) {
  while (n > 0) { //3pulseでスタート信号
    digitalWrite(digitalPIN, HIGH);
    delayMicroseconds(ON_reader);
    digitalWrite(digitalPIN, LOW);
    delayMicroseconds(OFF_reader);
    n --;
  }
  send_message(0);
}

void send_message(int n) {
  if (n == 8){
    if(Serial.available() == 0) {//ラスト
      crc_pulse(crc()); //data長さは　pdata
      //これをもとに信号を作成する必要あり 16bitの信号になるはず 
      pData = 0;
      Serial.println("");
      send_end(3);
    }else{
      Serial.println("");
      pData ++;
    }
  } else {
    switch (Data[pData]>>(7-n) & 1) { //0回シフト 下1bitで場合分けしているイメッジ 違う
      case 0:
        digitalWrite(digitalPIN, HIGH);
        delayMicroseconds(ON_0);
        digitalWrite(digitalPIN, LOW);
        delayMicroseconds(OFF_0);//
        Serial.print("0"); //確認
        send_message(n + 1);
        break;
      case 1:
        digitalWrite(digitalPIN, HIGH);
        delayMicroseconds(ON_1);
        digitalWrite(digitalPIN, LOW);
        delayMicroseconds(OFF_1);
        Serial.print("1"); //確認
        send_message(n + 1);
        break;
    }
  }
}

void send_end(int n) {
  while (n > 0) { //3pulseでスタート信号
    digitalWrite(digitalPIN, HIGH);
    delayMicroseconds(ON_reader);
    digitalWrite(digitalPIN, LOW);
    delayMicroseconds(OFF_reader);
    n --;
   }
 }


unsigned short crc(){
 unsigned int crc16 = 0xFFFFU; //初期値
 unsigned long i;
 int j;

 for ( i = 0 ; i <= pData ; i++ ){
   crc16 ^= (unsigned int)Data[i];
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

void crc_pulse(unsigned short crc16){
  for(int i =0;i<16;i++){
    switch(crc16>>(15-i) & 1){
       case 0:
        digitalWrite(digitalPIN, HIGH);
        delayMicroseconds(ON_0);
        digitalWrite(digitalPIN, LOW);
        delayMicroseconds(OFF_0);//
        Serial.print("0"); //確認
        break;
      case 1:
        digitalWrite(digitalPIN, HIGH);
        delayMicroseconds(ON_1);
        digitalWrite(digitalPIN, LOW);
        delayMicroseconds(OFF_1);
        Serial.print("1"); //確認
        break;      
    }
   }
}

