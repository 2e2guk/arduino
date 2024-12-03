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
  BTSerial.println("Test Message from Arduino!"); // HC-06으로 메시지 전송
  Serial.println("Test Message sent to Bluetooth"); // 시리얼 모니터로도 출력
  delay(1000); // 1초 간격으로 메시지 전송
}
