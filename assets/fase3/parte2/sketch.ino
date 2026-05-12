#include <WiFi.h>
#include <PubSubClient.h>
#include <DHT.h>

// =====================================================
// CARDIO IA - EDGE + MQTT
// ESP32 + DHT22 + Sensor BPM + LEDs
// =====================================================

// ========================
// WIFI WOKWI
// ========================

const char* ssid = "Wokwi-GUEST";
const char* password = "";

// ========================
// MQTT
// ========================

const char* mqttServer = "broker.hivemq.com";
const int mqttPort = 1883;

WiFiClient espClient;
PubSubClient client(espClient);

// ========================
// TÓPICOS MQTT
// ========================

const char* TOPICO_TEMP = "cardioia/temperatura";
const char* TOPICO_UMIDADE = "cardioia/umidade";
const char* TOPICO_BPM = "cardioia/bpm";
const char* TOPICO_ALERTA = "cardioia/alerta";
const char* TOPICO_STATUS = "cardioia/status";

// ========================
// PINOS
// ========================

#define DHT_PIN 4
#define HEART_PIN 34

#define LED_WIFI 18
#define LED_OFFLINE 19
#define LED_ALERTA 21

// ========================
// SENSOR DHT22
// ========================

#define DHTTYPE DHT22
DHT dht(DHT_PIN, DHTTYPE);

// ========================
// ESTRUTURA DOS DADOS
// ========================

struct SensorData {
  float temperatura;
  float umidade;
  int bpm;
  unsigned long timestamp;
};

// ========================
// BUFFER LOCAL
// ========================

const int MAX_DADOS = 20;
SensorData buffer[MAX_DADOS];
int totalDados = 0;

// ========================
// WIFI SIMULADO PARA EDGE
// ========================

bool wifiConectado = false;

// ========================
// CONTROLE DE TEMPO
// ========================

unsigned long ultimoToggleWifi = 0;
unsigned long ultimaLeitura = 0;

// =====================================================
// CONECTAR AO WIFI REAL DO WOKWI
// =====================================================

void conectarWiFiReal() {
  Serial.print("Conectando ao WiFi");

  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println();
  Serial.println("WiFi real conectado ao Wokwi!");
}

// =====================================================
// CONECTAR AO MQTT
// =====================================================

void conectarMQTT() {
  while (!client.connected()) {
    Serial.print("Conectando ao broker MQTT... ");

    String clientId = "cardioia-esp32-";
    clientId += String(random(0xffff), HEX);

    if (client.connect(clientId.c_str())) {
      Serial.println("conectado!");
      client.publish(TOPICO_STATUS, "ESP32 conectado ao MQTT");
    } else {
      Serial.print("falhou. Codigo: ");
      Serial.println(client.state());
      delay(2000);
    }
  }
}

// =====================================================
// PUBLICAR DADOS NO MQTT
// =====================================================

void publicarMQTT(float temperatura, float umidade, int bpm, unsigned long timestamp) {
  char tempStr[10];
  char umiStr[10];
  char bpmStr[10];

  dtostrf(temperatura, 4, 2, tempStr);
  dtostrf(umidade, 4, 2, umiStr);
  sprintf(bpmStr, "%d", bpm);

  client.publish(TOPICO_TEMP, tempStr);
  client.publish(TOPICO_UMIDADE, umiStr);
  client.publish(TOPICO_BPM, bpmStr);

  if (bpm > 120) {
    client.publish(TOPICO_ALERTA, "ALERTA CARDIACO");
  } else {
    client.publish(TOPICO_ALERTA, "NORMAL");
  }

  Serial.println("Dados publicados via MQTT:");
  Serial.print("Temp: ");
  Serial.print(temperatura);
  Serial.print(" C | Umidade: ");
  Serial.print(umidade);
  Serial.print(" % | BPM: ");
  Serial.print(bpm);
  Serial.print(" | Timestamp: ");
  Serial.println(timestamp);
}

// =====================================================
// SALVAR LOCALMENTE
// =====================================================

void salvarLocal(float temp, float umi, int bpm) {
  if (totalDados < MAX_DADOS) {
    buffer[totalDados] = {
      temp,
      umi,
      bpm,
      millis()
    };

    totalDados++;
  } else {
    for (int i = 1; i < MAX_DADOS; i++) {
      buffer[i - 1] = buffer[i];
    }

    buffer[MAX_DADOS - 1] = {
      temp,
      umi,
      bpm,
      millis()
    };
  }

  Serial.println("Dados armazenados localmente.");
}

// =====================================================
// SINCRONIZAR DADOS SALVOS
// =====================================================

void sincronizarDados() {
  Serial.println("\n===== SINCRONIZANDO DADOS SALVOS =====");

  if (totalDados == 0) {
    Serial.println("Nenhum dado pendente para sincronizar.");
    return;
  }

  for (int i = 0; i < totalDados; i++) {
    publicarMQTT(
      buffer[i].temperatura,
      buffer[i].umidade,
      buffer[i].bpm,
      buffer[i].timestamp
    );

    delay(300);
  }

  totalDados = 0;

  Serial.println("Sincronizacao concluida.");
  Serial.println("Buffer limpo.");
}

// =====================================================
// SETUP
// =====================================================

void setup() {
  Serial.begin(115200);
  delay(1000);

  Serial.println("=================================");
  Serial.println("INICIANDO CARDIO IA - MQTT");
  Serial.println("=================================");

  dht.begin();

  pinMode(LED_WIFI, OUTPUT);
  pinMode(LED_OFFLINE, OUTPUT);
  pinMode(LED_ALERTA, OUTPUT);

  digitalWrite(LED_WIFI, LOW);
  digitalWrite(LED_OFFLINE, HIGH);
  digitalWrite(LED_ALERTA, LOW);

  conectarWiFiReal();

  client.setServer(mqttServer, mqttPort);
  conectarMQTT();

  Serial.println("Sistema iniciado.");
}

// =====================================================
// LOOP PRINCIPAL
// =====================================================

void loop() {
  if (!client.connected()) {
    conectarMQTT();
  }

  client.loop();

  // =========================================
  // ALTERNA WIFI SIMULADO A CADA 15 SEGUNDOS
  // =========================================

  if (millis() - ultimoToggleWifi > 15000) {
    wifiConectado = !wifiConectado;

    Serial.println("\n=================================");

    if (wifiConectado) {
      Serial.println("WiFi SIMULADO CONECTADO");

      digitalWrite(LED_WIFI, HIGH);
      digitalWrite(LED_OFFLINE, LOW);

      client.publish(TOPICO_STATUS, "ONLINE");

      sincronizarDados();

    } else {
      Serial.println("WiFi SIMULADO DESCONECTADO");

      digitalWrite(LED_WIFI, LOW);
      digitalWrite(LED_OFFLINE, HIGH);

      client.publish(TOPICO_STATUS, "OFFLINE");
    }

    ultimoToggleWifi = millis();
  }

  // =========================================
  // LEITURA DOS SENSORES A CADA 5 SEGUNDOS
  // =========================================

  if (millis() - ultimaLeitura > 5000) {
    float temperatura = dht.readTemperature();
    float umidade = dht.readHumidity();

    int leituraPot = analogRead(HEART_PIN);
    int bpm = map(leituraPot, 0, 4095, 50, 140);

    if (bpm > 120) {
      digitalWrite(LED_ALERTA, HIGH);
      Serial.println("!!! ALERTA CARDIACO !!!");
    } else {
      digitalWrite(LED_ALERTA, LOW);
    }

    Serial.println("\n===== NOVA LEITURA =====");

    Serial.print("Temperatura: ");
    Serial.print(temperatura);
    Serial.println(" C");

    Serial.print("Umidade: ");
    Serial.print(umidade);
    Serial.println(" %");

    Serial.print("BPM: ");
    Serial.println(bpm);

    if (wifiConectado) {
      publicarMQTT(temperatura, umidade, bpm, millis());
    } else {
      salvarLocal(temperatura, umidade, bpm);
    }

    ultimaLeitura = millis();
  }
}