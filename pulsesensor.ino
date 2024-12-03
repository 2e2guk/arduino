#include <PulseSensorPlayground.h>

PulseSensorPlayground pulseSensor;

void setup() {
  Serial.begin(9600);
  pulseSensor.analogInput(A0); // 센서 핀이 A0에 연결
  pulseSensor.setThreshold(550); // 신호 감지 임계값 설정

  if (pulseSensor.begin()) {
    Serial.println("Pulse sensor initialized successfully!");
  } else {
    Serial.println("Pulse sensor initialization failed!");
  }
}

void loop() {
  int bpm = pulseSensor.getBeatsPerMinute();
  if (pulseSensor.sawStartOfBeat()) {
    Serial.print("Heart Rate: ");
    Serial.println(bpm);
  }
  delay(20); // 데이터 안정성을 위한 짧은 지연
}
