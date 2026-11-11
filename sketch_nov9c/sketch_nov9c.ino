#include <HTTPClient.h>
#include <WiFi.h>        // Substitui a ESP8266WiFi.h exigida para o NodeMCU, adaptando para a ESP32
#include <DHT.h>         // Biblioteca para o sensor DHT11
#include <SFE_BMP180.h>  // Biblioteca para o sensor BMP180
#include <Wire.h>        // Biblioteca para comunicação I2C

// --- Configurações da aplicação e de rede ---
const char* ssid = "brisa-1278550";           // Substitua pelo nome do seu Wi-Fi
const char* pass = "gvb7r7us";          // Substitua pela senha do seu Wi-Fi
const char* server = "api.thingspeak.com";       // Servidor da aplicação
String apiKey = "OG5OE54KCDVIGOE8";        // Substitua pela sua chave API de gravação do ThingSpeak
WiFiClient client;                               // Cria um cliente para conexão à internet

// --- Mapeamento de Pinos (Ajustado para ESP32) ---
// Na ESP32-WROOM, usamos a numeração dos GPIOs em vez das portas 'D' do ESP8266
#define DHTPIN 4         // GPIO 4 conectado ao DHT11
#define DHTTYPE DHT11    
#define SENSOR_AR 34     // GPIO 34 (pino apenas de leitura analógica recomendado na ESP32)
#define SENSOR_CHUVA 5   // GPIO 5 conectado ao Sensor de Chuva
#define LED_CHUVA 18     // GPIO 18 para o LED de alerta de chuva
#define BUZZER_CHUVA 19  // GPIO 19 para o Buzzer Piezo

// --- Objetos ---
DHT dht(DHTPIN, DHTTYPE);
SFE_BMP180 sensorP;

// --- Configurações dos Sensores ---
#define ALTITUDE 262.0

// Parâmetros de calibração do MQ-135
const float m = -0.3376;
const float b = 0.7165;
const float R0 = 10.55;  

// --- Variáveis Globais ---
float umidade = 0.0;
float temp_dht = 0.0;
double temp_bmp = 0.0;
double pressao_abs = 0.0;
double pressao_relativa = 0.0;
float ppm = 0.0;
int leituraChuva = 0;

// --- Controle de Tempo (millis) ---
unsigned long tempoAnterior = 0;
const unsigned long intervaloLeitura = 20000; // Alterado para 20s. O ThingSpeak na versão gratuita requer um intervalo mínimo de 15 segundos entre envios.

// --- Função: Leitura DHT11 ---
void lerDHT() {
  umidade = dht.readHumidity();
  temp_dht = dht.readTemperature();
}

// --- Função: Leitura MQ-135 ---
void lerQualidadeAr() {
  float sensor_volt = analogRead(SENSOR_AR) * (3.3 / 4095.0); // ESP32 possui ADC de 12 bits (4095), diferente dos 10 bits (1023) do ESP8266
  if (sensor_volt > 0) {
    float Rs = ((5.0 * 10.0) / sensor_volt) - 10.0;
    float razao = Rs / R0;
    float ppm_log = (log10(razao) - b) / m;
    ppm = pow(10, ppm_log);
  }
}

// --- Função: Leitura BMP180 ---
void lerPressao() {
  char status = sensorP.startTemperature();
  if (status != 0) {
    delay(status);
    status = sensorP.getTemperature(temp_bmp);
    if (status != 0) {
      status = sensorP.startPressure(3);
      if (status != 0) {
        delay(status);
        status = sensorP.getPressure(pressao_abs, temp_bmp);
        if (status != 0) {
          pressao_relativa = sensorP.sealevel(pressao_abs, ALTITUDE);
        }
      }
    }
  } else {
    Serial.println("[ERRO] Falha na leitura do BMP180.");
  }
}

// --- Função: Leitura e Alarme de Chuva ---
void lerEControlarChuva() {
  leituraChuva = digitalRead(SENSOR_CHUVA);

  if (leituraChuva == LOW) { 
    digitalWrite(LED_CHUVA, HIGH);
    // tone(BUZZER_CHUVA, 1500); // OBS: A função tone() nativa pode requerer a biblioteca ESP32Servo ou uso do ledcWrite na ESP32. Comentei para garantir que o código compile apenas na placa isolada.
  } else { 
    digitalWrite(LED_CHUVA, LOW);
    // noTone(BUZZER_CHUVA); 
  }
}

void setup() {
  Serial.begin(115200);
  delay(500);

  Serial.println("\n================================================");
  Serial.println("ESTAÇÃO METEOROLÓGICA - THINGSPEAK (ESP32-WROOM)");
  Serial.println("================================================");

  // Inicializa conexão Wi-Fi
  Serial.print("Conectando à rede WiFi: "); 
  Serial.println(ssid); 
  WiFi.begin(ssid, pass); 
  while (WiFi.status() != WL_CONNECTED) { 
    delay(500); 
    Serial.print("."); 
  }
  Serial.println("\nConectado à rede WiFi!"); 
  Serial.print("Endereço de IP: ");
  Serial.println(WiFi.localIP());

  dht.begin();
  if (sensorP.begin()) {
    Serial.println("[OK] Sensor BMP180 inicializado.");
  } else {
    Serial.println("[ERRO] Falha ao inicializar o sensor BMP180!");
  }

  // Configuração dos Pinos
  pinMode(SENSOR_AR, INPUT);
  pinMode(SENSOR_CHUVA, INPUT);
  pinMode(LED_CHUVA, OUTPUT);
  pinMode(BUZZER_CHUVA, OUTPUT);

  // Estado inicial das saídas
  digitalWrite(LED_CHUVA, LOW);
  
  Serial.println("Iniciando leituras...\n");
}

void loop() {
  unsigned long tempoAtual = millis();

  // Monitora a chuva continuamente para acionar o alarme local
  lerEControlarChuva();

  // Envia dados para o ThingSpeak a cada 20 segundos
  if (tempoAtual - tempoAnterior >= intervaloLeitura) {
    tempoAnterior = tempoAtual;

    lerDHT();
    lerQualidadeAr();
    lerPressao();

    // DADOS MOCK PARA TESTE SEM SENSORES
    umidade = 65.5;
    temp_dht = 28.3;
    pressao_relativa = 1012.5;
    pressao_abs = 998.0;
    ppm = 405.2;
    leituraChuva = HIGH; // Simula "Sem Chuva"

    // Mostra na Serial local
    Serial.println("----------------------------------------------");
    Serial.print("Temperatura (DHT11): "); Serial.print(temp_dht, 1); Serial.println(" °C");
    Serial.print("Umidade Relativa:    "); Serial.print(umidade, 1); Serial.println(" %");
    Serial.print("Pressão Absoluta:    "); Serial.print(pressao_abs, 1); Serial.println(" hPa");
    Serial.print("Pressão Relativa:    "); Serial.print(pressao_relativa, 1); Serial.println(" hPa");
    Serial.print("Qualidade do Ar:     "); Serial.print(ppm, 1); Serial.println(" PPM");
    Serial.print("Status de Chuva:     "); Serial.println(leituraChuva);
    Serial.println("----------------------------------------------");

    // --- Lógica de envio para o ThingSpeak usando HTTPClient ---
    if (WiFi.status() == WL_CONNECTED) {
      HTTPClient http;
      
      // Monta a string de dados no padrão exato que o ThingSpeak exige
      String dados = "api_key=" + apiKey + 
                     "&field1=" + String(umidade) + 
                     "&field2=" + String(temp_dht) + 
                     "&field3=" + String(pressao_relativa) + 
                     "&field4=" + String(pressao_abs) + 
                     "&field5=" + String(ppm) + 
                     "&field6=" + String(leituraChuva);
      
      // Inicia a conexão com o endpoint oficial
      http.begin("http://api.thingspeak.com/update");
      http.addHeader("Content-Type", "application/x-www-form-urlencoded");
      
      // Envia os dados (POST) e captura a resposta do servidor
      int httpResponseCode = http.POST(dados);
      
      if (httpResponseCode > 0) {
        Serial.print("Resposta do servidor (HTTP Code): ");
        Serial.println(httpResponseCode);
        
        if (httpResponseCode == 200) {
          Serial.println("SUCESSO: Dados salvos no gráfico do ThingSpeak!\n");
        } else {
          Serial.println("ERRO: O servidor recusou os dados. Verifique sua API Key.\n");
        }
      } else {
        Serial.print("ERRO CRÍTICO de rede HTTP: ");
        Serial.println(httpResponseCode);
      }
      
      http.end(); // Libera os recursos da placa
    } else {
      Serial.println("ERRO: Wi-Fi desconectado!\n");
    }
  }
}