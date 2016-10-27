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
#define N 20


#define PULSE_DRIVE_DURATION    200
#define PULSE_ZERO_DURATION     800
#define PULSE_ONE_DURATION     1600
#define PULSE_BREAK_DURATION   5000

int duration_counter=0;
int bit_index=0;
byte preamble=0;
byte recv_packet[N];
byte n_recv_packet=0;

void recvPacket(){
  
  int sig;
  int psig=analogRead(CT_SIGNAL);
  byte this_bit;
  
  n_recv_packet=0;
  while(1){
    sig=analogRead(CT_SIGNAL);
    if(sig-psig>150){
      //check duration_counter
      //Serial.print("duration_counter:");
      //Serial.println(duration_counter);
      if(duration_counter<5){
        // Do Nothing (destroy garbage)
      }else if(duration_counter<13){
        //this_bit=0;
        if(++bit_index>=5){
          byte byte_index=(bit_index-5)>>3;
          recv_packet[byte_index]>>=1;
        }
      }else if(duration_counter<26){
        //this_bit=1;
        if(++bit_index>=5){
          byte byte_index=(bit_index-5)>>3;
          recv_packet[byte_index]>>=1;
          recv_packet[byte_index]|=0x80;
        }
      }else{
      //  this_bit=2;   // start -- detected
        bit_index=0;
        n_recv_packet=0;
      }
      duration_counter=0;
    }
    psig=sig;

   if(++duration_counter>5000 || bit_index>=255){
        
      // break detected 
      if(bit_index>0){
        digitalWrite(LED_RX,HIGH);
        
//        if(!((bit_index-4)&0x07)){
//          n_recv_packet=(bit_index-5)>>3;
//          
//          if(n_recv_packet==4){
//            unsigned short recv_packet_crc=crc(recv_packet,3);
//            if(recv_packet[3]==byte(recv_packet_crc)){
//              Serial.print("ID="); Serial.print(recv_packet[0]);
//              Serial.print("; V="); Serial.print(recv_packet[1]);
//              Serial.print("; T="); Serial.println(recv_packet[2]);
//            }
//          }
//        }
        unsigned short recv_packet_crc=crc(recv_packet,N-1);
        if(recv_packet[N-1]==byte(recv_packet_crc)){ 
          for(int i = 0;i<N;i++){
          Serial.println(recv_packet[i]);
          }
        }       
        digitalWrite(LED_RX,LOW);

        bit_index=0;
        break; 
      }
      duration_counter=5000; 
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
  recvPacket();
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

