#include <wiring_private.h>
#define analogPIN 3

int value; //diff
int prev_val = 0;
int t = 0;

int ascii[8]; //2byte *8 16byte //文字なら010までは固定っぽい //5*8で40 hello用
int pointer = 0; //1khzだと最初読み取れないので1　ふつうは0
int flag = 0; //最初の確認
unsigned short crc16 = 0xFFFFU; //

void setup() {
  Serial.begin(9600);
  cbi(ADCSRA, ADPS2); //もともと cbi  adps0 prescale (set cbi 0 sbi 1)
  sbi(ADCSRA, ADPS1); //         sbi
  cbi(ADCSRA, ADPS0); //         cbi
  for(int i =0;i<8;i++){
    ascii[i] = 0;
  }
}

void loop() { //tの間隔　400 2000 400 2400 400 1600のとき　180-230 180 230
  value = analogRead(analogPIN);
  t++;
  if (value - prev_val >300) { 
     // これがないと1kHzはむりっぽい
    Serial.println(t);
    delayMicroseconds(50);
    if (60<= t && t<70) { //初期化する感じ この値は周期やcbiの設定によっsて変化してしまう
      //pointer = 0; //最後のパルス受信したときに格納先のポインターを初期化しておく
    }else if(50<= t && t<60 ){ //へんな1を殺す
       ascii[pointer] = 0; //1or0なら格納
       delayMicroseconds(100);
       //Serial.print(ascii[pointer]);
       pointer ++;
    }else if(t<=120 && t>=95){
       ascii[pointer] = 1; //1or0なら格納
       delayMicroseconds(100);
       //Serial.print(ascii[pointer]); //  1/Serial.begin() * (8*文字 + 1 + 1 ) たとえば9600で1文字は1ms程度
       pointer ++;
    }
    t =0;
  }
   if(pointer > 7){
    int len;
    Serial.print("flag:");
    Serial.println(flag);
     char c = ascii[0] * 128 + ascii[1] * 64 + ascii[2] * 32 + ascii[3] * 16 + ascii[4] * 8 + ascii[5] * 4 + ascii[6] * 2 + ascii[7];
     if(flag == 0){
       len = c; //これでlenにdatalength取得
       Serial.print("len:");
       Serial.println(len);
       flag = 1;
       pointer=0; 
     }else if(flag == 1 && len >0){
       len -- ;
       Serial.println(c);
       pointer = 0;
       crc(c) ; //この計算結果を把握しておく
     }else{
       pointer = 0;
       flag ++; //これを回数にとりあえず利用してみる
       check(c); //error判定 再送？？ 2回入りたい
       flag = 0; //戻しておく
       }
     }
  prev_val = value;
}

//crcを再帰的に計算するイメッジ
void crc(char c) {
 int j;
   crc16 ^= (unsigned int)c;
   for ( j = 0 ; j < 8 ; j++ ){
     if ( crc16 & 0x0001 ){
       crc16 = (crc16 >> 1) ^ 0xA001;
     }else{
       crc16 >>= 1;
     }
   }
}

void check(char c){
  if(flag == 2){
    if(((crc16 >> 8) ^ (0x0000 ^ c)) != 0x0000){
      Serial.println("error!");
    }
  }
  if(flag == 3){
    if(((crc16  <<8)^ ((0x0000 ^c) <<8)) != 0x0000){
      Serial.println("error!");
    }
  }
}
