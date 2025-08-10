#include <WiFi.h>
#include <HTTPClient.h>
#include "DHT.h"

// WiFi 정보
const char* ssid = "cse";
const char* password = "cse272727";

// DHT 센서 설정
#define DHTPIN 22
#define DHTTYPE DHT11
DHT dht(DHTPIN, DHTTYPE);

// 모터 드라이버 핀 설정
#define MOTOR_IN1 25
#define MOTOR_IN2 26

// 서버 주소
const char* serverName = "http://192.168.1.9:5000/sensor";

// 식물 ID 설정
const char* plant_id = "mint";  // basil, mint, tomato 중 선택

void setup() {
  Serial.begin(115200);
  delay(1000);

  dht.begin();

  // 모터 드라이버 핀 설정
  pinMode(MOTOR_IN1, OUTPUT);
  pinMode(MOTOR_IN2, OUTPUT);
  digitalWrite(MOTOR_IN1, LOW);
  digitalWrite(MOTOR_IN2, LOW);  // 펌프 OFF 초기화

  // WiFi 연결
  WiFi.begin(ssid, password);
  Serial.print("📡 WiFi 연결 중...");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\n✅ WiFi 연결 완료!");
}

void loop() {
  if (WiFi.status() == WL_CONNECTED) {
    float h = dht.readHumidity();
    float t = dht.readTemperature();

    if (isnan(h) || isnan(t)) {
      Serial.println("❌ 센서 읽기 실패!");
      delay(5000);
      return;
    }

    // JSON 데이터에 plant_id 포함
    String jsonData = "{\"plant_id\": \"" + String(plant_id) + "\", \"temperature\": " + String(t, 1) + ", \"humidity\": " + String(h, 1) + "}";
    Serial.println("📤 전송할 데이터: " + jsonData);

    HTTPClient http;
    http.begin(serverName);
    http.addHeader("Content-Type", "application/json");
    int httpResponseCode = http.POST(jsonData);

    if (httpResponseCode > 0) {
      String response = http.getString();
      Serial.println("✅ 서버 응답: " + response);
    } else {
      Serial.println("❌ 전송 실패, 코드: " + String(httpResponseCode));
    }
    http.end();

    // 💧 펌프 작동 조건
    if (h < 60.0) {
      Serial.println("💧 습도 낮음 - 펌프 작동");
      digitalWrite(MOTOR_IN1, HIGH);
      digitalWrite(MOTOR_IN2, LOW);
      delay(10000);  // 10초 작동
      digitalWrite(MOTOR_IN1, LOW);
      digitalWrite(MOTOR_IN2, LOW);
      Serial.println("🛑 펌프 정지");
    }

  } else {
    Serial.println("❌ WiFi 끊김 - 재연결 시도...");
    WiFi.begin(ssid, password);
  }

  delay(30000);  // 30초마다 측정 및 전송
}


