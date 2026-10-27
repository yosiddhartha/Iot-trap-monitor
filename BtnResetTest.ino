#include <M5Stack.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include "config.h"

const char* ssid     = WIFI_SSID;
const char* password = WIFI_PASSWORD;

const char* webhook_url = DISCORD_WEBHOOK_URL;

// スイッチを繋ぐピン（G26）
const int SW_PIN = 26; 

// M5Stackの「Aボタン」が内部で繋がっているピン番号
const int MY_BUTTON_A_PIN = 39; 

// ★定期報告（生存確認）の間隔設定（秒単位）
// テスト用に10分（600秒）。本番（山の中）では1日＝86400秒などに変更してください。
#define TIME_TO_SLEEP_NORMAL  600   
#define S_TO_US_FACTOR        1000000ULL 

void setup() {
  // 画面ON、スピーカーOFF、SD OFFで初期化
  M5.begin(true, false, false);
  pinMode(SW_PIN, INPUT_PULLUP);
  pinMode(MY_BUTTON_A_PIN, INPUT_PULLUP); 
  delay(100); 

  // M5Stackが起きた理由を取得
  esp_sleep_wakeup_cause_t wakeup_reason = esp_sleep_get_wakeup_cause();

  // ─── ここから下は、通信や画面表示を行うメイン処理 ───

  M5.Lcd.fillScreen(BLACK);
  M5.Lcd.setTextSize(2);
  M5.Lcd.setTextColor(WHITE);
  M5.Lcd.setCursor(10, 20);
  M5.Lcd.print("Connecting Wi-Fi...");

  WiFi.begin(ssid, password);
  int retry_count = 0;
  while (WiFi.status() != WL_CONNECTED && retry_count < 30) {
    delay(500);
    M5.Lcd.print(".");
    retry_count++;
  }

  // 1. 【現場復帰】もし「Aボタン」で起こされた場合
  if (wakeup_reason == ESP_SLEEP_WAKEUP_EXT1) {
    M5.Lcd.fillScreen(GREEN);
    M5.Lcd.setCursor(40, 100);
    M5.Lcd.print("MANUAL RESET!");
    sendDiscordNotification("【現場復帰】ボタン操作によりシステムが再起動しました。監視を再開します。");
  }
  // 2. 【罠作動】もし「洗濯ばさみスイッチ（G26）」で起こされた場合
  else if (wakeup_reason == ESP_SLEEP_WAKEUP_EXT0) {
    M5.Lcd.fillScreen(RED);
    M5.Lcd.setCursor(40, 100);
    M5.Lcd.print("TRAP TRIGGERED!");
    sendDiscordNotification("【🚨実戦警報】罠が作動しました！確認してください。");
  } 
  // 3. 【定期報告】もし「タイマー」で起こされた場合
  else if (wakeup_reason == ESP_SLEEP_WAKEUP_TIMER) {
    M5.Lcd.fillScreen(BLUE);
    M5.Lcd.setCursor(40, 100);
    M5.Lcd.print("Sending Heartbeat...");
    sendDiscordNotification("【定期報告】異常なし。罠センサーは生存しています。");
  } 
  // 4. 初回起動時（電源を入れた直後など）
  else {
    sendDiscordNotification("【システム始動】監視を開始し、ディープスリープに入ります。");
  }

  // 通知が確実に飛び立つまでの猶予を持たせて冬眠へ
  delay(2000); 
  goToSleep();
}

// 😴 冬眠関数
void goToSleep() {
  M5.Lcd.fillScreen(BLACK);
  M5.Lcd.setBrightness(0); // バックライト消灯

  // いつでもAボタン（G39）が押されたら起きるようにしておく
  esp_sleep_enable_ext1_wakeup((1ULL << MY_BUTTON_A_PIN), ESP_EXT1_WAKEUP_ALL_LOW);

  // いま現在、下敷きが挟まっている（ネジが離れてHIGH）の時だけ「タイマー」と「ピン」をセットする。
  if (digitalRead(SW_PIN) == HIGH) {
    // 正常待機中なので、10分おきの定期報告タイマーをセット
    esp_sleep_enable_timer_wakeup(TIME_TO_SLEEP_NORMAL * S_TO_US_FACTOR);
    
    // 正常待機中なので、罠が弾けた瞬間（LOW）を監視するピン目覚ましをセット
    esp_sleep_enable_ext0_wakeup((gpio_num_t)SW_PIN, 0);
  }
  // ネジがくっついている（LOW）の時は、上のif文を通らないため、
  // タイマーもピンもセットされず、Aボタンの目覚ましだけがセットされた状態になります！

  // 冬眠スタート
  esp_deep_sleep_start();
}

void loop() {
  // 使用しません
}

void sendDiscordNotification(String message) {
  if (WiFi.status() == WL_CONNECTED) {
    HTTPClient http;
    http.begin(webhook_url);
    http.addHeader("Content-Type", "application/json");
    String jsonPayload = "{\"content\": \"" + message + "\"}";
    int httpResponseCode = http.POST(jsonPayload);
    http.end();
  }
}