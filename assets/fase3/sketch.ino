#include <DHT.h>

// =====================================================
// CARDIO IA - EDGE COMPUTING
// ESP32 + DHT22 + Sensor BPM + LEDs
// =====================================================

// ========================
// PINOS
// ========================

#define DHT_PIN 4
#define HEART_PIN 34

// LEDs
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
// WIFI SIMULADO
// ========================

bool wifiConectado = false;

// ========================
// CONTROLE DE TEMPO
// ========================

unsigned long ultimoToggleWifi = 0;
unsigned long ultimaLeitura = 0;

// =====================================================
// SALVAR LOCALMENTE
// =====================================================

void salvarLocal(float temp, float umi, int bpm) {

  // Se ainda houver espaço no buffer
  if (totalDados < MAX_DADOS) {

    buffer[totalDados] = {
      temp,
      umi,
      bpm,
      millis()
    };

    totalDados++;

  } else {

    // Remove o dado mais antigo
    for (int i = 1; i < MAX_DADOS; i++) {
      buffer[i - 1] = buffer[i];
    }

    // Adiciona novo dado no final
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
// ENVIAR PARA NUVEM
// =====================================================

void enviarParaNuvem() {

  Serial.println("\n===== SINCRONIZANDO DADOS =====");

  for (int i = 0; i < totalDados; i++) {

    Serial.print("Temp: ");
    Serial.print(buffer[i].temperatura);

    Serial.print(" C | Umidade: ");
    Serial.print(buffer[i].umidade);

    Serial.print(" % | BPM: ");
    Serial.print(buffer[i].bpm);

    Serial.print(" | Timestamp: ");
    Serial.println(buffer[i].timestamp);
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
  Serial.println("INICIANDO CARDIO IA");
  Serial.println("=================================");

  // Inicializa DHT22
  dht.begin();

  // Configura LEDs
  pinMode(LED_WIFI, OUTPUT);
  pinMode(LED_OFFLINE, OUTPUT);
  pinMode(LED_ALERTA, OUTPUT);

  // Estado inicial
  digitalWrite(LED_WIFI, LOW);
  digitalWrite(LED_OFFLINE, HIGH);
  digitalWrite(LED_ALERTA, LOW);

  Serial.println("Sistema iniciado.");
}

// =====================================================
// LOOP PRINCIPAL
// =====================================================

void loop() {

  // =========================================
  // ALTERNA WIFI A CADA 15 SEGUNDOS
  // =========================================

  if (millis() - ultimoToggleWifi > 15000) {

    wifiConectado = !wifiConectado;

    Serial.println("\n=================================");

    if (wifiConectado) {

      Serial.println("WiFi CONECTADO");

      // LEDs
      digitalWrite(LED_WIFI, HIGH);
      digitalWrite(LED_OFFLINE, LOW);

      // Envia dados acumulados
      enviarParaNuvem();

    } else {

      Serial.println("WiFi DESCONECTADO");

      // LEDs
      digitalWrite(LED_WIFI, LOW);
      digitalWrite(LED_OFFLINE, HIGH);
    }

    ultimoToggleWifi = millis();
  }

  // =========================================
  // LEITURA DOS SENSORES
  // =========================================

  if (millis() - ultimaLeitura > 5000) {

    float temperatura = dht.readTemperature();
    float umidade = dht.readHumidity();

    // Leitura do potenciômetro
    int leituraPot = analogRead(HEART_PIN);

    // Conversão para BPM
    int bpm = map(leituraPot, 0, 4095, 50, 140);

    // =========================================
    // ALERTA CARDÍACO
    // =========================================

    if (bpm > 120) {

      digitalWrite(LED_ALERTA, HIGH);

      Serial.println("!!! ALERTA CARDIACO !!!");

    } else {

      digitalWrite(LED_ALERTA, LOW);
    }

    // =========================================
    // EXIBE DADOS
    // =========================================

    Serial.println("\n===== NOVA LEITURA =====");

    Serial.print("Temperatura: ");
    Serial.print(temperatura);
    Serial.println(" C");

    Serial.print("Umidade: ");
    Serial.print(umidade);
    Serial.println(" %");

    Serial.print("BPM: ");
    Serial.println(bpm);

    // =========================================
    // EDGE COMPUTING
    // =========================================

    if (wifiConectado) {

      Serial.println("Dados enviados diretamente para nuvem.");

    } else {

      salvarLocal(
        temperatura,
        umidade,
        bpm
      );
    }

    ultimaLeitura = millis();
  }
}