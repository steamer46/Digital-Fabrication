/* 참고: esp->노트북 Serial 전송 / 노트북->esp 는 esp32.write(b'1\n')
 * 날씨소스: WeatherAPI.com (forecast.json)  ※ https 접속
 */

#include <WiFi.h>
#include <HTTPClient.h>
#include <WiFiClientSecure.h>   // ★ https 접속용
#include <ArduinoJson.h>
#include <Adafruit_NeoPixel.h>

//wifi 정보
const char* ssid = "yang";
const char* password = "aa102672";

// WeatherAPI.com 홍대 좌표 예보 (현재날씨 + 오늘 강수확률)
const String url = "https://api.weatherapi.com/v1/forecast.json?key=b8779c449f7e435ba4c104820263006&q=37.55,126.92&days=1&aqi=no&alerts=no";

// ===== 매트릭스 설정 =====
#define LED_PIN     4
#define MATRIX_W    8
#define MATRIX_H    8
#define NUMPIXELS   (MATRIX_W * MATRIX_H)
#define BRIGHTNESS  30
#define FRAME_MS    200

bool SERPENTINE      = false;
bool FLIP_VERTICAL   = false;
bool FLIP_HORIZONTAL = false;

Adafruit_NeoPixel strip(NUMPIXELS, LED_PIN, NEO_GRB + NEO_KHZ800);

int currentIcon = 0;
int phase = 0;
unsigned long lastFrame = 0;
unsigned long lastShownTime = 0;         // 마지막 표시 시각
const unsigned long DISPLAY_MS = 30000;  // 30초 후 꺼짐

uint32_t codeToColor(byte c) {
  switch (c) {
    case 1: return strip.Color(255, 190,   0);
    case 2: return strip.Color(255,  80,   0);
    case 3: return strip.Color(200, 200, 210);
    case 4: return strip.Color(110, 110, 120);
    case 5: return strip.Color( 45,  45,  55);
    case 6: return strip.Color(  0,  90, 255);
    default: return 0;
  }
}

// 아이콘 (0맑음 1구름많음 2흐림 3비 4눈 5오류)
const byte ICONS[6][8][8] = {
  { {0,0,2,0,0,2,0,0},{0,0,0,1,1,0,0,0},{2,0,1,1,1,1,0,2},{0,1,1,1,1,1,1,0},
    {0,1,1,1,1,1,1,0},{2,0,1,1,1,1,0,2},{0,0,0,1,1,0,0,0},{0,0,2,0,0,2,0,0} },
  { {1,1,0,0,0,0,0,0},{1,1,0,0,3,3,0,0},{0,0,0,3,3,3,3,0},{0,0,3,3,3,3,3,4},
    {0,4,4,3,3,3,4,4},{0,4,5,5,5,5,5,5},{0,0,5,5,5,5,5,0},{0,0,0,0,0,0,0,0} },
  { {0,0,0,0,0,0,0,0},{0,0,0,3,3,3,0,0},{0,0,3,3,3,3,3,0},{0,3,3,3,3,3,3,3},
    {4,4,4,4,4,4,4,4},{5,5,5,5,5,5,5,5},{0,5,5,5,5,5,5,0},{0,0,0,0,0,0,0,0} },
  { {0,0,0,3,3,3,0,0},{0,0,3,3,3,3,3,0},{0,4,4,4,4,4,4,4},{0,5,5,5,5,5,5,5},
    {0,0,5,5,5,5,5,0},{0,0,0,0,0,0,0,0},{0,6,0,0,6,0,0,6},{6,0,0,6,0,0,6,0} },
  { {0,0,0,3,3,3,0,0},{0,0,3,3,3,3,3,0},{0,4,4,4,4,4,4,4},{0,5,5,5,5,5,5,5},
    {0,0,5,5,5,5,5,0},{0,0,0,0,0,0,0,0},{0,3,0,0,3,0,0,3},{3,0,0,3,0,0,3,0} },
  { {0,0,2,2,2,2,0,0},{0,2,2,0,0,2,2,0},{0,0,0,0,0,2,2,0},{0,0,0,0,2,2,0,0},
    {0,0,0,2,2,0,0,0},{0,0,0,2,2,0,0,0},{0,0,0,2,2,0,0,0},{0,0,0,2,2,0,0,0} }
};

uint16_t XY(uint8_t x, uint8_t y) { //matrix 좌우 반전
  if (FLIP_HORIZONTAL) x = MATRIX_W - 1 - x;
  if (FLIP_VERTICAL)   y = MATRIX_H - 1 - y;
  if (SERPENTINE && (y % 2 == 1))
    return y * MATRIX_W + (MATRIX_W - 1 - x);
  return y * MATRIX_W + x;
}

void drawGrid(const byte g[8][8]) { //matrix 순서 배열
  strip.clear();
  for (int y = 0; y < MATRIX_H; y++)
    for (int x = 0; x < MATRIX_W; x++)
      strip.setPixelColor(XY(x, y), codeToColor(g[y][x]));
  strip.show();
}

void drawPrecip(int idx, int phase) { //matrix 배치
  byte g[8][8];
  for (int y = 0; y < 8; y++)
    for (int x = 0; x < 8; x++)
      g[y][x] = (y < 5) ? ICONS[idx][y][x] : 0;
  if (idx == 3) {
    const int cols[6] = {1,4,7,0,3,6};
    const int off[6]  = {0,1,2,1,2,0};
    for (int i = 0; i < 6; i++) g[5 + ((phase + off[i]) % 3)][cols[i]] = 6;
  } else {
    const int cols[5] = {1,4,6,2,7};
    const int off[5]  = {0,1,2,1,0};
    int slow = phase / 2;
    for (int i = 0; i < 5; i++) g[5 + ((slow + off[i]) % 3)][cols[i]] = 3;
  }
  drawGrid(g);
}

void drawCurrent() {
  if (currentIcon == 3 || currentIcon == 4) drawPrecip(currentIcon, phase);
  else drawGrid(ICONS[currentIcon]);
}

// ★ WeatherAPI.com condition code → 아이콘 (0맑음 1구름많음 2흐림 3비 4눈 5오류)
int weatherToIcon(int code) {
  if (code == 1000) return 0;                                          // 맑음
  if (code == 1003) return 1;                                          // 구름많음
  if (code == 1006 || code == 1009 || code == 1030 ||
      code == 1135 || code == 1147) return 2;                          // 흐림/안개
  // 눈 / 진눈깨비(강)/ 얼음
  if (code == 1066 || code == 1114 || code == 1117 || code == 1207 ||
      (code >= 1210 && code <= 1225) || code == 1237 || code == 1252 ||
      (code >= 1255 && code <= 1264) || code == 1279 || code == 1282) return 4;
  // 비 / 이슬비 / 소나기 / 천둥 / 약한 진눈깨비
  if (code == 1063 || code == 1069 || code == 1072 || code == 1087 ||
      (code >= 1150 && code <= 1201) || code == 1204 ||
      (code >= 1240 && code <= 1249) || code == 1273 || code == 1276) return 3;
  return 5;                                                            // 그 외 오류
}


void checkWeather() {
  if (WiFi.status() != WL_CONNECTED) WiFi.reconnect();

  WiFiClientSecure client;
  client.setInsecure();          // 인증서 검증 생략 (https 간편 접속)

  HTTPClient http;
  http.useHTTP10(true);
  http.begin(client, url);       // https + client
  http.setConnectTimeout(10000);
  http.setTimeout(10000);
  int httpCode = http.GET();

  if (httpCode <= 0) {
    Serial.print("ERROR: 연결실패 code=");
    Serial.println(httpCode);
    Serial.println(http.errorToString(httpCode));
    currentIcon = 5; phase = 0; drawCurrent();
    http.end();
    return;
  }

  // 응답을 문자열로 받아서 앞부분 확인 (디버깅용)
  String payload = http.getString();
  http.end();
  Serial.println("=== 받은 응답 ===");
  Serial.println(payload.substring(0, 300));   // 앞 300자만 출력
  Serial.println("=================");

  JsonDocument doc;
  DeserializationError error = deserializeJson(doc, payload);

  if (error) {
    Serial.print("ERROR:JSON파싱실패 ");
    Serial.println(error.c_str());
    currentIcon = 5; phase = 0; drawCurrent();
    return;
  }

  // WeatherAPI 필드에서 뽑기
  float temp = doc["current"]["temp_c"].as<float>();                       // 온도(℃)
  int weatherCode = doc["current"]["condition"]["code"].as<int>();          // 날씨 코드
  int rainProb = doc["forecast"]["forecastday"][0]["day"]["daily_chance_of_rain"].as<int>(); // 오늘 강수확률(%)

  // 날씨 → 아이콘
  currentIcon = weatherToIcon(weatherCode);
  phase = 0;
  drawCurrent();

  bool isRaining = (currentIcon == 3);   // 아이콘이 비(3)면 1

  // 시리얼 전송 (WEATHER:온도,강수확률,비여부)
  Serial.print("WEATHER: ");
  Serial.print(temp);
  Serial.print(",");
  Serial.print(rainProb);
  Serial.print(",");
  Serial.println(isRaining ? "1" : "0");
}

void setup(){
  Serial.begin(115200);

  strip.begin();
  strip.setBrightness(BRIGHTNESS);
  strip.clear();        // 꺼진 상태로 시작
  strip.show();

  WiFi.begin(ssid, password);
  Serial.println("STATUS: WiFi연결중");
  while (WiFi.status() != WL_CONNECTED) { delay(100); }
  Serial.println("STATUS: WiFi연결완료");

  // checkWeather();   ← 켜자마자 표시 안 함 (지우거나 주석)
}
  

void loop() {
  if (Serial.available() > 0) {
    String cmd = Serial.readStringUntil('\n');
    cmd.trim();

    if (cmd == "1") {              // 사람 감지 → 날씨 확인
      checkWeather();
      lastShownTime = millis();    // 켜진 시각 기록
    }
    else if (cmd.length() == 1 && cmd[0] >= '2' && cmd[0] <= '7') {
      currentIcon = cmd[0] - '2';  // 테스트용 수동표시
      phase = 0;
      drawCurrent();
      lastShownTime = millis();
    }
  }

  // 표시 후 30초 지나면 끄기
  if (lastShownTime != 0 && (millis() - lastShownTime >= DISPLAY_MS)) {
    strip.clear();
    strip.show();
    lastShownTime = 0;             // 껐으니 초기화
  }

  // 비/눈 애니메이션 (켜져 있을 때만)
  if (lastShownTime != 0 && (currentIcon == 3 || currentIcon == 4) &&
      (millis() - lastFrame >= FRAME_MS)) {
    lastFrame = millis();
    phase = (phase + 1) % 6;
    drawCurrent();
  }
}
