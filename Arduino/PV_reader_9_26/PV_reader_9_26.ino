#include <wiring_private.h>
#define analogPIN 3
#define N 13 // 1 + 10 + 3

int value; //diff
int prev_val = 0;
int t = 0;

 //int ascii[8]; //2byte *8 16byte //文字なら010までは固定っぽい //5*8で40 hello用
bool packet[N][8]; //intで保管するとめんどそう
int p = 0;
int pointer = 0;
int status = 0;


unsigned short crc16 = 0xFFFFU; //

void setup() {
  Serial.begin(115200);
  cbi(ADCSRA, ADPS2); //もともと cbi  adps0 prescale (set cbi 0 sbi 1)
  sbi(ADCSRA, ADPS1); //         sbi
  cbi(ADCSRA, ADPS0); //         cbi
  for(int i =0;i<N;i++){
    for(int j = 0;j<8;j++){
      packet[i][j] = 0;
    }
  }
}

void loop() { //tの間隔　400 2000 400 2400 400 1600のとき　180-230 180 230

  value = analogRead(analogPIN);
  t++;
  /*
  //if(value - prev_val > 300){
  //Serial.print("t:");
  //Serial.print(t-1);
  //Serial.print(" value:");
  if(value > 500){
  //Serial.print(t);
  //Serial.print(",");
  Serial.println(value);
  t = 0;
}
//}
*/
if (value - prev_val >300) {
  if(status <= 2) {
    delayMicroseconds(50);
    if (80<= t && t<110) { //初期化する感じ この値は周期やcbiの設定によっsて変化してしまう
      Serial.println("初期値");
      status ++;
    }
    }else if(status > 2){
      if(50<= t && t< 100 ){ //へんな1を殺す
        //ascii[pointer] = 0; //1or0なら格納
        packet[p][pointer] = 0;
        delayMicroseconds(100);
        //Serial.print(ascii[pointer]);
        pointer ++;
        //Serial.print("0: ");
        //Serial.println(pointer);
        p_inc();

        }else if(t<=140 && t>=100){
          //ascii[pointer] = 1; //1or0なら格納
          packet[p][pointer] = 1;
          delayMicroseconds(100);
          //Serial.print(ascii[pointer]); //  1/Serial.begin() * (8*文字 + 1 + 1 ) たとえば9600で1文字は1ms程度
          pointer ++;
          //Serial.print("1: ");
          //Serial.println(pointer);
          p_inc();
        }
        else if(t>200){  //試し
          show(); //文字見せる リセット
          for(int i =0;i<N;i++){
            for(int j =0;j<8;j++){
              packet[i][j] = 0;
            }
          }
          p = 0;
          pointer = 0;
          status = 0;
        }
        t = 0;
      }

      prev_val = value;
    }
  }

void p_inc(){
  if(pointer >7){
    p ++;
    pointer = 0;
  }
}

void show(){
  //初手長さ
  char c =  packet[0][0] * 128 + packet[0][1] * 64 + packet[0][2] * 32 + packet[0][3] * 16 + packet[0][4] * 8 + packet[0][5] * 4 + packet[0][6] * 2 + packet[0][7];
  int len = c ;
  Serial.print("len:");
  Serial.println(len);
  p++;
  while(len >0){
    c = packet[p][0] * 128 + packet[p][1] * 64 + packet[p][2] * 32 + packet[p][3] * 16 + packet[p][4] * 8 + packet[p][5] * 4 + packet[p][6] * 2 + packet[p][7];
    Serial.print("data:");
    Serial.println(c);
    crc(c);
    p++;
    len--;
  }
  check();
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

void check(){
  for(int i = 0;i<16;i++){
    if((crc16 >> (15-i)) ^ packet[p][i%8] == 1){
      Serial.println("error!");
    }
    if(i == 7){
      p++;
    }
  }
}
