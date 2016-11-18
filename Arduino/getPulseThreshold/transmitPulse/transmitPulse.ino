// パルスの閾値判断（送信用）

// ピンレイアウトの定義
#define CT_SIGNAL A0
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

void transmitPulse(){
  digitalWrite(L_DRIVE, HIGH);
  delayMicroseconds(PULSE_DRIVE_DURATION);
  digitalWrite(L_DRIVE, LOW);
}

void setup() {
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

void loop() {
  
  digitalWrite(LED_TX, HIGH);

  // パルス作成
  transmitPulse();
  // 0か1か区別するためのdurationをランダムに
  if(random(2)){
    delayMicroseconds(PULSE_ONE_DURATION);
  }else{
    delayMicroseconds(PULSE_ZERO_DURATION);
  }
  
  digitalWrite(LED_TX, LOW);

  delay(1000);
}
