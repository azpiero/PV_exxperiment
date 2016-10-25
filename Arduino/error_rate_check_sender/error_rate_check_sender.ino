#include <wiring_private.h> //analogread高速化
#define digitalPIN 9              // LEDはピン13に接続

int ON_reader = 200; // 400 2000 / 400 2400 /400 16000
int OFF_reader = 3000;
int ON_end = 200;
int OFF_end = 2000;
int ON_1 = 200;
int OFF_1 = 1300;
int ON_0 = 200;
int OFF_0 = 700; //お試し

char Data[10]; //とりあえず10文字で
int pData = 0;
char crc[2]; //これもなんとかしたいな
int flag = 0; //最初に到達したSerialか否か いらなそう

void setup() {
  Serial.begin(9600);
  pinMode(digitalPIN, OUTPUT);
}

void loop() {
  if (Serial.available() > 0) { //Serial.available()の返り値がデータのバイト数 charなら1byte 64までいける
    Data[pData] = Serial.read();
    delayMicroseconds(10000);
    pData ++;
    if(Serial.available() == 0){
      flag = 1;
    }
  }
  if(flag == 1){
    char len = (pData + '0') ; //文字数を取得
    for(int j=0;j<100;j++){
     make_markpulse(ON_reader,OFF_reader); //
     make_pulse(len,8); //lengthを送るため
     for(int i = 0;i<pData;i++){
       make_pulse(Data[i],8); //
      }
     add_checksum();
     delay(500); //delayなのに単位がmsecじゃなさそう
   }
   delay(3000); //delayなのに単位がmsecじゃなさそう
  }
}

void make_markpulse(int ON, int OFF) {
  for(int n = 0; n<3; n++){
    digitalWrite(digitalPIN, HIGH);
    delayMicroseconds(ON);
    digitalWrite(digitalPIN, LOW);
    delayMicroseconds(OFF);
  }
}

void cal_crc(){
 unsigned int crc16 = 0xFFFFU; //初期値
 unsigned long i;
 int j;
 for ( i = 0 ; i < pData ; i++ ){
   crc16 ^= (unsigned int)Data[i];
   for ( j = 0 ; j < 8 ; j++ ){
     if ( crc16 & 0x0001 ){
       crc16 = (crc16 >> 1) ^ 0xA001;
     }else{
       crc16 >>= 1;
     }
   }
 }
 crc[0] = (crc16 >> 8) & 0xFFU;
 crc[1] = crc16 & 0xFFU  ; //これが微妙
}

void make_pulse(char data, int n){ //
  switch (data >>(n-1) & 1) { //0回シフト 下1bitで場合分けしているイメッジ 違う
    case 0:
      digitalWrite(digitalPIN, HIGH);
      delayMicroseconds(ON_0);
      digitalWrite(digitalPIN, LOW);
      delayMicroseconds(OFF_0);//
      Serial.print("0"); //確認
      if(n > 1) make_pulse(data,n - 1);
      break;
    case 1:
      digitalWrite(digitalPIN, HIGH);
      delayMicroseconds(ON_1);
      digitalWrite(digitalPIN, LOW);
      delayMicroseconds(OFF_1);
      Serial.print("1"); //確認
      if(n > 1) make_pulse(data,n - 1);
      break;
  }
}

void add_checksum(){
  cal_crc();
  make_pulse(crc[0],8); //いまいち釈然としないけど笑
  make_pulse(crc[1],8);
  make_markpulse(ON_end,OFF_end);
  Serial.println();
}
