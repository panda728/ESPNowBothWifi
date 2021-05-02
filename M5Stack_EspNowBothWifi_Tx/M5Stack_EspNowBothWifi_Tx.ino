#include <esp_now.h>
#include <WiFi.h>
#include <M5Stack.h>

#define WIFI_CHANNEL_ESPNOW 1

const char *ssidSoftAP = "ESP32AP-WiFi";
const char *passSoftAP = "esp32apwifi";

uint8_t remoteMac[] = {0x44, 0x44, 0x44, 0x44, 0x44, 0x41};

esp_now_peer_info_t peerRx;
const esp_now_peer_info_t *peerRxNode = &peerRx;

const uint8_t maxDataFrameSize = 250;
uint8_t dataToSend[maxDataFrameSize];
byte cnt = 0;
int dataSent = 0;
long timers[3];

void setup()
{
  Serial.begin(115200);
  M5.begin();
  M5.Power.begin();
  M5.Lcd.fillScreen(TFT_BLACK);
  M5.Lcd.setRotation(1);
  M5.Lcd.setCursor(0, 10);
  M5.Lcd.setTextColor(WHITE);
  M5.Lcd.setTextSize(2);
  M5.Lcd.println("ESPNow/Wifi Example Tx");
  
  reset_wifi();
  start_espnow();
  
  M5.Lcd.print("AP MAC: "); M5.Lcd.println(WiFi.softAPmacAddress());
  M5.Lcd.print("Wifi channel: "); M5.Lcd.println(WiFi.channel());
}

void reset_wifi() {
  WiFi.persistent(false);
  WiFi.disconnect();
  WiFi.mode(WIFI_OFF);
  delay(10);
}

void start_espnow() {
  WiFi.mode(WIFI_STA);
  WiFi.softAP(ssidSoftAP, passSoftAP, WIFI_CHANNEL_ESPNOW);
  Serial.print("AP MAC: "); Serial.println(WiFi.softAPmacAddress());
  Serial.print("Wifi channel: "); Serial.println(WiFi.channel());
  if (esp_now_init() == ESP_OK) {
    Serial.println("ESP NOW INIT!");
  } else {
    Serial.println("ESP NOW INIT FAILED....");
  }
  memcpy( &peerRx.peer_addr, &remoteMac, 6 );
  peerRx.channel = WIFI_CHANNEL_ESPNOW;
  peerRx.encrypt = 0;
  peerRx.ifidx = ESP_IF_WIFI_AP;

  if ( esp_now_add_peer(peerRxNode) == ESP_OK)  {
    Serial.println("Added Peer!");
  }
  esp_now_register_send_cb(OnDataSent);
  esp_now_register_recv_cb(OnDataRecv);
}

void loop()
{
  M5.update();
  // ボタンを押したら送信
  if ( M5.BtnA.wasPressed() ) {
    M5.Lcd.fillScreen(TFT_BLACK);
    M5.Lcd.setCursor(0, 10);
    for (cnt = 0; cnt < maxDataFrameSize; cnt++)  {
      dataToSend[cnt]++;
    }
    if ( esp_now_send(peerRx.peer_addr, dataToSend, maxDataFrameSize) == ESP_OK) {
      timers[0] = micros();
      M5.Lcd.printf("\r\nSuccess Sent Value->\t%d", dataToSend[0]);
    }
  }
  delay(1);
}

void OnDataSent(const uint8_t *mac_addr, esp_now_send_status_t status)
{
  M5.Lcd.print("\r\nLast Packet Send Status:\t");
  M5.Lcd.print(status == ESP_NOW_SEND_SUCCESS ? "Delivery Success" : "Delivery Fail");
}

void OnDataRecv(const uint8_t *mac_addr, const uint8_t *data, int data_len)
{
  if (data[0] == dataToSend[0])
  {
    timers[1] = micros();
    timers[2] = timers[1] - timers[0];
    M5.Lcd.printf("\r\nReceived\t%d Bytes val\t%d\t in %ld micros", data_len, data[0], timers[2]);
  }
  else
  {
    M5.Lcd.printf("\r\nReceived\t%d Bytes\t%d", data_len, data[0]);
  }
}
