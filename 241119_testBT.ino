#include <SoftwareSerial.h>

#define BT_TX 7 // HC-06 모듈의 RXD 핀에 연결
#define BT_RX 8 // HC-06 모듈의 TXD 핀에 연결

SoftwareSerial BTSerial(BT_RX, BT_TX); // 소프트웨어 시리얼 객체 생성

void setup() {
  Serial.begin(9600);     // 시리얼 모니터용 기본 시리얼 통신
  BTSerial.begin(9600);   // HC-06 블루투스 모듈과의 통신
  Serial.println("HC-06 블루투스 테스트 시작");
}

void loop() {
  // 시리얼 모니터에서 입력받은 데이터를 블루투스로 전송
  if (Serial.available()) {
    char c = Serial.read();
    BTSerial.write(c);
  }

  // 블루투스에서 받은 데이터를 시리얼 모니터로 출력
  if (BTSerial.available()) {
    char c = BTSerial.read();
    Serial.write(c);
  }

  // 블루투스로 자동 메시지 전송
  BTSerial.println("Test Message from Arduino!");
  delay(1000);
}
