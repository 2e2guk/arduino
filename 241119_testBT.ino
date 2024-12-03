// 라이브러리 포함
#include <SoftwareSerial.h> // HC-06 블루투스 모듈 사용 헤더
#include <PulseSensorPlayground.h> // 심박센서
#include <Adafruit_MLX90614.h> // 적외선온도센서
// 타이머 헤더. 

unsigned long conditionStartTime = 0; // 조건 충족 시작 시간 기록
bool conditionMet = false; // 조건 충족 여부

void setup() {
  // 센서 설정 -> 초기화.(재작성필요)
  initializeHeartRateSensor(); // 심박센서
  initializeTemperatureSensor(); // 체온센서
  // 블루투스 모듈 초기화.
  // 타이머 설정. 
}
void loop() {
  unsigned long currentMillis = millis(); // 현재 시간. 

  // 8개의 임계값 변수 조정. -> 초기값 설정하고 사용자에 맞게. 
  int low_danger_heartrate_low = 0;
  int low_danger_heartrate_high = 0;

  int high_danger_heartrate_low = 0;
  int high_danger_heartrate_high = 0;

  int low_danger_temp_low = 0;
  int low_danger_temp_high = 0;

  int high_danger_temp_low = 0;
  int high_danger_temp_high = 0;

  // 사용자 or 119 알림을 보내줄 임계값
  int treshold = 1;

  float heartRate = readHeartRate(low_danger_heartrate_low, low_danger_heartrate_high, ,high_danger_heartrate_low, high_danger_heartrate_high);
  float bodyTemp = readBodyTemperature(low_danger_temp_low, low_danger_temp_high, high_danger_temp_low, high_danger_temp_high);
  
  // 위험 임계값 확인 -> 수정
  float alertval = 0.7 * heartRate + 0.3 * bodyTemp;

  /*if(alertval >= treshold) {
    if(//timer > 1min) {
      alert119();
    }
  } else if(alertval < treshold && alertval > 0) {
    alertUser(); // 사용자에게 알림. 
  }*/
  if (alertval >= treshold) {
    if (!conditionMet) {
      conditionMet = true;
      conditionStartTime = currentMillis; // 조건 충족 시작 시간 기록
    } else {
      unsigned long elapsedTime = currentMillis - conditionStartTime; // 경과 시간 계산
      if (elapsedTime >= 60000) { // 1분(60000ms)이 넘었는지 확인
        alert119();
      }
    }
  } else {
    // 조건이 충족되지 않으면 상태 초기화
    conditionMet = false;
    conditionStartTime = 0;
  }
  delay(5000); // 5초 주기 데이터 수집
}
float readHeartRate(int low_low, int low_high, int high_low, int high_high) {
  // 사용자 심박수 읽음 -> by 심박수센서
  float currentHeartRate = getHeartRateFromSensor();

  float normalizedValue = 0.0;

  if (currentHeartRate < low_low) {
    // 생명이 위험한 수준으로 낮은 심박수
    normalizedValue = 1.0;
  } else if (currentHeartRate >= low_low && currentHeartRate < low_high) {
    // 위험한 수준으로 낮은 심박수
    normalizedValue = (low_high - currentHeartRate) / (low_high - low_low);
  } else if (currentHeartRate >= low_high && currentHeartRate <= high_low) {
    // 정상 범위의 심박수
    normalizedValue = 0.0;
  } else if (currentHeartRate > high_low && currentHeartRate <= high_high) {
    // 위험한 수준으로 높은 심박수
    normalizedValue = (currentHeartRate - high_low) / (high_high - high_low);
  } else if (currentHeartRate > high_high) {
    // 생명이 위험한 수준으로 높은 심박수
    normalizedValue = 1.0;
  }

  return normalizedValue; // 0.0에서 1.0 사이의 값 반환

}
float readBodyTemperature(int low_low, int low_high, int high_low, int high_high) {
  // 사용자 체온 읽음 -> by 적외선센서
  float currentBodyTemp = getBodyTempFromSensor();

  float normalizedValue = 0.0;

  if (currentBodyTemp < low_low) {
    // 생명이 위험한 수준으로 낮은 체온
    normalizedValue = 1.0;
  } else if (currentBodyTemp >= low_low && currentBodyTemp < low_high) {
    // 위험한 수준으로 낮은 체온
    normalizedValue = (low_high - currentBodyTemp) / (low_high - low_low);
  } else if (currentBodyTemp >= low_high && currentBodyTemp <= high_low) {
    // 정상 범위의 체온
    normalizedValue = 0.0;
  } else if (currentBodyTemp > high_low && currentBodyTemp <= high_high) {
    // 위험한 수준으로 높은 체온
    normalizedValue = (currentBodyTemp - high_low) / (high_high - high_low);
  } else if (currentBodyTemp > high_high) {
    // 생명이 위험한 수준으로 높은 체온
    normalizedValue = 1.0;
  }

  return normalizedValue; // 0.0에서 1.0 사이의 값 반환
}
void alertUser() {
  // 사용자 알림 
  // 블루투스 모듈
}
void alert119() {
  // 119에 알림. 
  // 블루투스 모듈 + 보호자에게 1차로 알림 가고, 보호자 통해서 119로. or 119에 앱이나 웹사이트 요청. 
}
float getHeartRateFromSensor(){
  // 센서로 사용자 심박수 읽음. 
}
float getBodyTempFromSensor() {
  // 센서로 사용자 체온 읽음. 
}
/*
1. 약간 위험 범위에서 유지될 경우에 대한 논의 필요 -> 보호자에게 어떻게 알려줄건가.
2. 가중치가 진짜 필요한가?
3. sigmoid 를 쓸수 있는가? + sigmoid보다 나은 함수가 있나?
*/
