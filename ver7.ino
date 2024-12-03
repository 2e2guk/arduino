// 필요한 라이브러리 포함
#include <SoftwareSerial.h>         // HC-06 블루투스 모듈 통신용
#include <PulseSensorPlayground.h>  // 심박수 센서용
#include <Adafruit_MLX90614.h>      // 적외선 온도 센서용
#include <TinyGPS++.h>              // GPS 모듈용

// 센서 객체 생성
PulseSensorPlayground pulseSensor;
Adafruit_MLX90614 mlx = Adafruit_MLX90614();
TinyGPSPlus gps;

// HC-06 블루투스 모듈은 시리얼 통신 사용
#define BT_TX 7
#define BT_RX 8
SoftwareSerial BTSerial(BT_RX, BT_TX); // RX | TX

// GPS 모듈은 시리얼 통신 사용
#define GPS_TX 4
#define GPS_RX 3
SoftwareSerial gpsSerial(GPS_RX, GPS_TX); // RX | TX

// 타이밍 변수
unsigned long conditionStartTime = 0; // 알림 조건 시작 시간 기록
bool conditionMet = false;            // 알림 조건 충족 여부

// 임계값 (사용자가 필요에 따라 조정해야 함)
float low_danger_temp_low = 35.0;    // A: 매우 위험한 낮은 체온
float low_danger_temp_high = 36.0;   // B: 약간 위험한 낮은 체온
float high_danger_temp_low = 37.5;   // C: 약간 위험한 높은 체온
float high_danger_temp_high = 39.0;  // D: 매우 위험한 높은 체온

int low_danger_heartrate_low = 40;   // E: 매우 위험한 낮은 심박수
int low_danger_heartrate_high = 50;  // F: 약간 위험한 낮은 심박수
int high_danger_heartrate_low = 100; // G: 약간 위험한 높은 심박수
int high_danger_heartrate_high = 120;// H: 매우 위험한 높은 심박수

// 알림 임계값
float threshold = 0.7; // 알림을 위한 임계값 (필요에 따라 조정)

// 보호자 정보
String guardianName = "이강욱";
String guardianPhoneNumber = "01062842718";

// 센서 읽기 값을 저장할 변수
int currentHeartRate = 0;
float currentBodyTemp = 0.0;
float latitude = 0.0;
float longitude = 0.0;

void setup() {
  // 시리얼 통신 초기화
  Serial.begin(9600);
  BTSerial.begin(9600); // HC-06 블루투스 모듈 보드레이트
  gpsSerial.begin(9600); // GPS 모듈 보드레이트

  // 센서 초기화
  initializeHeartRateSensor();
  initializeTemperatureSensor();
}

// 상태 플래그 추가
bool guardianNotified = false; // 보호자에게 알림이 전송되었는지 여부

void loop() {
  unsigned long currentMillis = millis(); // 현재 시간 가져오기

  // GPS 데이터 읽기
  while (gpsSerial.available() > 0) {
    gps.encode(gpsSerial.read());
  }

  if (gps.location.isUpdated()) {
    latitude = gps.location.lat();
    longitude = gps.location.lng();
    Serial.print("위도: ");
    Serial.println(latitude, 6);
    Serial.print("경도: ");
    Serial.println(longitude, 6);
  }

  // 5초마다 체온과 심박수 읽기 및 출력
  if (currentMillis % 5000 < 50) {
    // 체온 읽기
    currentBodyTemp = getBodyTempFromSensor();
    Serial.print("체온: ");
    Serial.println(currentBodyTemp);

    // 심박수 읽기
    int bpm = pulseSensor.getBeatsPerMinute();
    if (bpm > 0) { // 유효한 심박수만 출력
      currentHeartRate = bpm;
    }
    Serial.print("심박수: ");
    Serial.println(currentHeartRate);
  }

  // 정규화된 값 계산 (비선형 함수 사용)
  float heartRateNormalized = sigmoid(normalizeHeartRate(currentHeartRate, low_danger_heartrate_low, low_danger_heartrate_high, high_danger_heartrate_low, high_danger_heartrate_high));
  float bodyTempNormalized = sigmoid(normalizeBodyTemperature(currentBodyTemp, low_danger_temp_low, low_danger_temp_high, high_danger_temp_low, high_danger_temp_high));

  // 가중치를 사용하여 알림 값 계산
  float alertval = 0.7 * heartRateNormalized + 0.3 * bodyTempNormalized;

  // 알림 조건 확인
  if (alertval >= threshold) {
    if (!conditionMet) {
      conditionMet = true;
      conditionStartTime = currentMillis; // 조건 충족 시작 시간 기록
    } else {
      unsigned long elapsedTime = currentMillis - conditionStartTime;
      if (elapsedTime >= 1000) { // 조건이 1분 이상 지속되면 1분 60000
        alert119();
        sendGuardianAlertAfter119();
        guardianNotified = false; // 보호자 알림 상태 초기화
      }
    }
  } else if (alertval > 0 && alertval < threshold) {
    if (!guardianNotified) { // 보호자 알림이 전송되지 않은 경우
      alertGuardian();
      guardianNotified = true; // 알림 전송 상태를 업데이트
    }
    // 상태 초기화
    conditionMet = false;
    conditionStartTime = 0;
  } else {
    // 모든 조건이 충족되지 않으면 상태 초기화
    conditionMet = false;
    conditionStartTime = 0;
    guardianNotified = false; // 보호자 알림 상태 초기화
  }

  delay(50); // 센서 읽기 안정성을 위한 짧은 지연
}

void initializeHeartRateSensor() {
  // 심박수 센서 초기화
  pulseSensor.analogInput(A0); // 센서가 A0에 연결되었다고 가정
  pulseSensor.setThreshold(550);
  if (pulseSensor.begin()) {
    Serial.println("심박수 센서 초기화 성공.");
  } else {
    Serial.println("심박수 센서 초기화 실패.");
  }
}

void initializeTemperatureSensor() {
  // 체온 센서 초기화
  if (mlx.begin()) {
    Serial.println("체온 센서 초기화 성공.");
  } else {
    Serial.println("체온 센서 초기화 실패.");
  }
}

float getBodyTempFromSensor() {
  // 센서로부터 체온 읽기
  float tempC = mlx.readObjectTempC();
  return tempC;
}

float normalizeHeartRate(float heartRate, int low_low, int low_high, int high_low, int high_high) {
  float normalizedValue = 0.0;

  if (heartRate < low_low) {
    normalizedValue = -1.0;
  } else if (heartRate >= low_low && heartRate < low_high) {
    normalizedValue = -1.0 + 2.0 * (heartRate - low_low) / (low_high - low_low);
  } else if (heartRate >= low_high && heartRate <= high_low) {
    normalizedValue = 0.0;
  } else if (heartRate > high_low && heartRate <= high_high) {
    normalizedValue = 2.0 * (heartRate - high_low) / (high_high - high_low);
  } else if (heartRate > high_high) {
    normalizedValue = 1.0;
  }

  return normalizedValue; // -1.0에서 1.0 사이의 값 반환
}

float normalizeBodyTemperature(float bodyTemp, float low_low, float low_high, float high_low, float high_high) {
  float normalizedValue = 0.0;

  if (bodyTemp < low_low) {
    normalizedValue = -1.0;
  } else if (bodyTemp >= low_low && bodyTemp < low_high) {
    normalizedValue = -1.0 + 2.0 * (bodyTemp - low_low) / (low_high - low_low);
  } else if (bodyTemp >= low_high && bodyTemp <= high_low) {
    normalizedValue = 0.0;
  } else if (bodyTemp > high_low && bodyTemp <= high_high) {
    normalizedValue = 2.0 * (bodyTemp - high_low) / (high_high - high_low);
  } else if (bodyTemp > high_high) {
    normalizedValue = 1.0;
  }

  return normalizedValue; // -1.0에서 1.0 사이의 값 반환
}

float sigmoid(float x) {
  // 시그모이드 함수 적용
  return 1.0 / (1.0 + exp(-x * 5.0)); // 기울기 조정을 위해 5.0을 곱함
}

void alertGuardian() {
  String message = "ALERT_GUARDIAN\n";
  message += "심박수:" + String(currentHeartRate) + " BPM\n";
  message += "체온:" + String(currentBodyTemp) + " °C\n";
  message += "위치:" + String(latitude, 6) + "," + String(longitude, 6) + "\n";
  BTSerial.println(message);
  Serial.println("보호자에게 알림을 보냈습니다.");
}

void alert119() {
  String message = "ALERT_119\n";
  message += "심박수:" + String(currentHeartRate) + " BPM\n";
  message += "체온:" + String(currentBodyTemp) + " °C\n";
  message += "위치:" + String(latitude, 6) + "," + String(longitude, 6) + "\n";
  message += "보호자명:" + guardianName + "\n";
  message += "보호자연락처:" + guardianPhoneNumber + "\n";
  BTSerial.println(message);
  Serial.println("119에 긴급 알림을 보냈습니다.");
}

void sendGuardianAlertAfter119() {
  String message = "119에 긴급 신고가 접수되었습니다.\n";
  message += "접수 센터: 서울특별시 강남구 119 안전센터\n";
  message += "센터 위치: 서울특별시 강남구 테헤란로 123\n";
  BTSerial.println("AFTER_119\n" + message);
  Serial.println("보호자에게 119 신고 정보를 보냈습니다.");
}
