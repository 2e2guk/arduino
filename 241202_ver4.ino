void loop() {
  unsigned long currentMillis = millis(); // 현재 시간 가져오기

  // 심박수 데이터 연속적으로 읽기
  readHeartRateSensorData();

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

  // 5초마다 체온 읽기
  if (currentMillis % 5000 < 50) {
    currentBodyTemp = getBodyTempFromSensor();
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
      if (elapsedTime >= 60000) { // 조건이 1분 이상 지속되면
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
