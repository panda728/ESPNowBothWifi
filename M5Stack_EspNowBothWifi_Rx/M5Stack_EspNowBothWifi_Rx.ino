#include <esp_now.h>
#include <WiFi.h>
#include <M5Stack.h>

#define WIFI_CHANNEL_ESPNOW 1

const char *ssidSta = "ssid";
const char *passSta = "pass";

const char *ssidAP = "ESP32AP-WiFi";
const char *passAP = "esp32apwifi";

uint8_t remoteMac[] = {0x33, 0x33, 0x33, 0x33, 0x33, 0x31};

esp_now_peer_info_t peerTx;
const esp_now_peer_info_t *peerTxNode = &peerTx;

const byte maxDataFrameSize = 250;
uint8_t dataRx[maxDataFrameSize];
bool data_recieved = false;

void setup()
{
  Serial.begin(115200);
  M5.begin();
  M5.Power.begin();
  M5.Lcd.fillScreen(TFT_BLACK);
  M5.Lcd.setCursor(0, 10);
  M5.Lcd.setTextColor(WHITE);
  M5.Lcd.setTextSize(2);
  M5.Lcd.println("ESPNow/Wifi Example Rx");

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
void start_wifi() {
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssidSta, passSta);
  int maxTry = 10;
  while (WiFi.status() != WL_CONNECTED && maxTry > 0 ) {
    delay(1000);
    Serial.print(".");
    maxTry--;
  }
  Serial.println("WiFi connected..!");
  Serial.print("IP: ");  Serial.println(WiFi.localIP());
  Serial.print("MAC: "); Serial.println(WiFi.macAddress());
  Serial.print("Wifi channel: "); Serial.println(WiFi.channel());
}

void start_espnow() {
  WiFi.mode(WIFI_AP);
  WiFi.softAP(ssidAP, passAP, WIFI_CHANNEL_ESPNOW);
  Serial.print("AP MAC: "); Serial.println(WiFi.softAPmacAddress());
  Serial.print("Wifi channel: "); Serial.println(WiFi.channel());
  if (esp_now_init() == ESP_OK) {
    Serial.println("ESPNow Init Success!");
  } else {
    Serial.println("ESPNow Init Failed....");
  }

  memcpy( &peerTx.peer_addr, &remoteMac[0], 6 );
  peerTx.channel = WIFI_CHANNEL_ESPNOW;
  peerTx.encrypt = 0;
  peerTx.ifidx = ESP_IF_WIFI_AP;

  if ( esp_now_add_peer(peerTxNode) == ESP_OK) {
    Serial.println("Added peerNode!");
  }

  esp_now_register_recv_cb(OnDataRecv);
  esp_now_register_send_cb(OnDataSent);
}

void loop()
{
  delay(1);
  if (data_recieved) {
    M5.Lcd.clear(BLACK);
    M5.Lcd.setCursor(0, 0);
    M5.Lcd.printf("Received val %d", dataRx[0]); M5.Lcd.println("");
    Serial.printf("Received val %d", dataRx[0]);

    data_recieved = false;
    delay(10);
    reset_wifi();
    start_wifi();
    M5.Lcd.print("STA MAC: "); M5.Lcd.println(WiFi.macAddress());
    M5.Lcd.print("Wifi channel: "); M5.Lcd.println(WiFi.channel());
    //hogehoge
    start_espnow();
    M5.Lcd.print("AP MAC: "); M5.Lcd.println(WiFi.softAPmacAddress());
    M5.Lcd.print("Wifi channel: "); M5.Lcd.println(WiFi.channel());
  }
}

void OnDataRecv(const uint8_t *mac_addr, const uint8_t *data, int data_len)
{
  memcpy( dataRx, data, data_len );
  esp_err_t sendResult = esp_now_send(peerTx.peer_addr, dataRx, sizeof(dataRx));
  data_recieved = true;
}

void OnDataSent(const uint8_t *mac_addr, esp_now_send_status_t status)
{
  Serial.print("\r\nLast Packet Send Status:\t");
  Serial.println(status == ESP_NOW_SEND_SUCCESS ? "Delivery Success" : "Delivery Fail");
}
