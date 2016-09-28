//SRAM 2kbyte 

#include <wiring_private.h>
#define analogPIN 3
#define N 10

int value; //diff
int prev_val = 0;
int t = 0;

int ascii[N*8]; //2byte *8 16byte //文字なら010までは固定っぽい //5*8で40 hello用
int pointer = 0; //文字のasciiは初手0

void setup() {
  Serial.begin(9600);
  cbi(ADCSRA, ADPS2); //もともと cbi  adps0 prescale (set cbi 0 sbi 1)
  sbi(ADCSRA, ADPS1); //         sbi
  cbi(ADCSRA, ADPS0); //         cbi
  for(int i =0;i<N*8;i++){
    ascii[i] = 0;
  }
}

void loop() {
  value = (analogRead(analogPIN)) ;
  t++;
  if (value - prev_val > 200) { //500くらいのdiff
    delayMicroseconds(50);
    //Serial.println(t);
    if (75 <= t && t< 90) { //初期化する感じ この値は周期やcbiの設定によっsて変化してしまう
      pointer =0; //最後のパルス受信したときに格納先のポインターを初期化しておく
    }else if(40<t && t<75 ){ //へんな1を殺す
       ascii[pointer] = 0; //1or0なら格納
       Serial.print(ascii[pointer]);
       pointer ++;
    }else if(t<120 &&t>90){
       ascii[pointer] = 1; //1or0なら格納
       Serial.print(ascii[pointer]);
       pointer ++;
    }
    t =0;
  }
  prev_val = value;

  //ここが一定時間ごとに表示するタイプでいいと思います。

  if (t >10000) { //10000は試し
    for(int i =0;i<N;i++){
    char c = ascii[i*8] * 128 + ascii[i*8+1] * 64 + ascii[i*8+2] * 32 + ascii[i*8+3] * 16 + ascii[i*8+4] * 8 + ascii[i*8+5] * 4 + ascii[i*8+6] * 2 + ascii[i*8+7];
    Serial.print(c);
    }
    for(int i= 0;i<N*8;i++){
      ascii[i] = 0; //初期化
    }
    pointer = 0;
    Serial.println(" ");
    t =0; 
  }
}



