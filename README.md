# 🐾 IoT くくり罠 遠隔監視システム (IoT Trap Monitor)

※このコードはgeminiを使用したバイブコーディングにより作成しました。
M5Stack (ESP32) を活用し、野生鳥獣捕獲用くくり罠の作動状況を遠隔から監視・通知するIoTデバイスのファームウェアです。
罠が作動した際（または定時生存確認時）に、Wi-Fi/LTE経由で Discord へ自動通知を送信します。
センサーは「洗濯ばさみスイッチ」を自作して使用しています。
洗濯ばさみの挟まる面に穴をあけ、M5Stackのピンから伸ばした線をネジで止めます。その洗濯ばさみに下敷き等の絶縁体を挟みます。
罠が動作した際に下敷きが引っ張られて洗濯ばさみから抜けることで、接点がONになります。
自宅での検証は行いましたが、まだ狩猟にいくことができていないので実戦未投入です。

---

## 🌟 主な機能と特徴

- **即時作動検知・通知**  
  罠に設置したスイッチの切り替えを検知し、リアルタイムで Discord に通知メッセージを送信します。
- **省電力設計（Deep Sleep）**  
  バッテリー駆動時間を最大化するため、ESP32の **Deep Sleep 機能** を実装。検知時および定時復帰時のみ起動することで長時間の野外稼働を実現しています。
- **定時生存確認（ハートビート機能）**  
  デバイスが正常に動作しているか（バッテリー切れや通信障害が起きていないか）を確認するため、一定時間ごとに自動で起動して生存報告を行います。
- **機密情報の安全管理**  
  SSIDやAPIトークンなどの設定情報は `config.h` として分離し、リポジトリからは除外する安全な設計を採用しています。

---

## 🛠️ 技術スタック & ハードウェア

- **マイコンボード:** M5Stack Basic (ESP32)
- **開発言語:** C / C++ (Arduino IDE / VS Code)
- **通信 protocol:** HTTP / HTTPS (Discord Webhook API)
- **ネットワーク:** Wi-Fi (または SORACOM LTE-M モジュール等)
- **センサー:** くくり罠連動スイッチ (GPIOピン検知)

---

## 📁 フォルダ構成

```text
.
├── BtnResetTest.ino   # メインプログラムコード
├── config.sample.h    # 設定ファイルのテンプレート（Wi-Fi/Webhook用）
└── .gitignore         # config.h やビルド成果物の除外設定
```

## 🚀 セットアップ方法
1. リポジトリのクローン
```Bash

git clone [https://github.com/yosiddhartha/Iot-trap-monitor.git](https://github.com/yosiddhartha/Iot-trap-monitor.git)
```

2. 設定ファイルの作成
config.sample.h を複製して config.h を作成し、ご自身の環境に合わせて接続情報を入力してください。
```C++

// config.h
#define WIFI_SSID "YOUR_WIFI_SSID"
#define WIFI_PASSWORD "YOUR_WIFI_PASSWORD"
#define DISCORD_WEBHOOK_URL "YOUR_DISCORD_WEBHOOK_URL"
```

3. 書き込み
Arduino IDE や PlatformIO を使用して M5Stack へビルド・書き込みを行います。

## 💡 工夫したポイント（ポートフォリオ補足）
山林やフィールドでの運用を想定し、「いかにバッテリーを長持ちさせるか」「いかに誤作動・失報を防ぐか」に重きを置きました。単なる通知機能だけでなく、Deep Sleep からの復帰要因（esp_sleep_get_wakeup_cause）に応じた処理分岐や、定時通信による生存確認システムを組み込むことで、現場での実用性を高めています。

---
