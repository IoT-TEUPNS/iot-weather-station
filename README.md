# 🌰 Estação Meteorológica IoT para Monitoramento de Cajueiro

Projeto da disciplina de **Internet das Coisas (IoT)**: uma estação meteorológica baseada em **ESP32** que monitora as condições ambientais de uma plantação de **cajueiro** e envia os dados em tempo real para a nuvem (**ThingSpeak**), permitindo acompanhar o clima da lavoura e antecipar riscos como doenças fúngicas e perdas na floração.

> 🚧 **Status:** em desenvolvimento. A versão inicial foi escrita em Arduino/C++ e está sendo migrada para C e validada em protótipo físico.

---

## 📑 Sumário

- [Objetivo](#-objetivo)
- [Por que monitorar o cajueiro?](#-por-que-monitorar-o-cajueiro)
- [Condições ideais para o cultivo do caju](#-condições-ideais-para-o-cultivo-do-caju)
- [Como a estação monitora a plantação](#-como-a-estação-monitora-a-plantação)
- [Hardware utilizado](#-hardware-utilizado)
- [Mapeamento de pinos](#-mapeamento-de-pinos)
- [Arquitetura do sistema](#-arquitetura-do-sistema)
- [Campos do ThingSpeak](#-campos-do-thingspeak)
- [Bibliotecas necessárias](#-bibliotecas-necessárias)
- [Configuração e execução](#-configuração-e-execução)
- [Calibração dos sensores](#-calibração-dos-sensores)
- [Limitações conhecidas](#-limitações-conhecidas)
- [Melhorias futuras](#-melhorias-futuras)
- [Equipe](#-equipe)
- [Licença](#-licença)

---

## 🎯 Objetivo

Coletar continuamente dados de **temperatura, umidade, pressão atmosférica, qualidade do ar e presença de chuva** na área de cultivo de cajueiros, disponibilizando-os em um painel online e acionando alertas locais (LED e buzzer) quando há chuva.

## 🌱 Por que monitorar o cajueiro?

O cajueiro (*Anacardium occidentale*) é uma cultura típica do semiárido e do litoral do Nordeste brasileiro, muito sensível ao clima durante as fases de **floração e frutificação**. Chuvas e alta umidade nessa fase favorecem doenças como **antracnose, oídio e mofo-preto**, além de causar queda de flores e frutos. Monitorar o microclima da plantação ajuda o produtor a:

- Identificar períodos de **risco para doenças fúngicas**;
- Planejar **manejo, pulverizações e colheita** com base em dados reais;
- Acompanhar a **variação do clima** ao longo do ciclo da cultura;
- Detectar **fumaça/poluentes** no ar (ex.: queimadas próximas).

## 🌤️ Condições ideais para o cultivo do caju

Os valores abaixo são **referências gerais** da literatura agronômica (como as publicadas pela Embrapa Agroindústria Tropical) e podem variar conforme a variedade (ex.: cajueiro-anão precoce) e a região. Devem ser validados com um técnico agrícola.

| Fator | Condição ideal | Observações |
|---|---|---|
| **Temperatura** | Média anual entre **27 °C e 30 °C** (faixa favorável: ~22 °C a 32 °C) | Temperaturas muito baixas prejudicam a floração; calor extremo reduz a produtividade |
| **Umidade relativa do ar** | Em torno de **70% a 80%** | Umidade muito alta na floração favorece doenças fúngicas |
| **Precipitação** | **800 a 1.500 mm/ano**, bem distribuídos | Precisa de um **período seco de 5 a 6 meses** para florescer e frutificar bem |
| **Chuva na floração/frutificação** | **Ausente ou mínima** | Chuvas nessa fase causam queda de flores e favorecem antracnose e oídio |
| **Insolação** | Sol pleno, ~**2.000 a 2.800 h/ano** | A cultura é exigente em luminosidade |
| **Altitude** | Preferencialmente **abaixo de 600–700 m** | Melhor desempenho em regiões litorâneas e de baixa altitude |
| **Solo** | Profundo, **bem drenado**, arenoso ou areno-argiloso, pH entre ~**5,0 e 6,5** | Não tolera encharcamento |
| **Ventos** | Moderados | Ventos fortes podem causar queda de flores e frutos |

**Calendário típico no Nordeste:** floração entre ~agosto e dezembro e colheita entre ~setembro e janeiro, coincidindo com a estação seca.

## 📡 Como a estação monitora a plantação

Cada sensor da estação está ligado a uma necessidade da cultura:

| Sensor | Variável | Como ajuda no monitoramento do cajueiro |
|---|---|---|
| **DHT11** | Temperatura e umidade relativa | Verifica se o clima está dentro da faixa ideal e sinaliza risco de doenças fúngicas quando a umidade fica alta |
| **BMP180** | Pressão atmosférica (absoluta e ao nível do mar) | Queda brusca de pressão pode indicar **aproximação de chuva ou instabilidade**, útil para planejar o manejo |
| **Sensor de chuva** | Presença de chuva | Detecta chuva em tempo real, especialmente crítica na floração/frutificação; aciona **LED e buzzer** locais |
| **MQ-135** | Qualidade do ar (PPM estimado) | Indica presença de fumaça e poluentes, como queimadas no entorno |

### 🚦 Faixas de referência para alertas (sugestão)

Valores sugeridos para interpretar os dados no painel. Devem ser ajustados com orientação agronômica:

| Variável | ✅ Adequado | ⚠️ Atenção | 🔴 Crítico |
|---|---|---|---|
| Temperatura | 24–32 °C | 20–24 °C ou 32–35 °C | < 20 °C ou > 35 °C |
| Umidade relativa | 60–80% | 80–85% | > 85% na floração |
| Chuva | Sem chuva | Chuva fora da floração | Chuva na floração/frutificação |
| Pressão | Estável | Queda gradual | Queda brusca (possível chuva) |

## 🔧 Hardware utilizado

- 1× **ESP32-WROOM** (DevKit)
- 1× Sensor de temperatura e umidade **DHT11**
- 1× Sensor de pressão barométrica **BMP180** (I²C)
- 1× Sensor de qualidade do ar **MQ-135**
- 1× **Sensor de chuva** (saída digital)
- 1× **LED** de alerta + resistor
- 1× **Buzzer piezo**
- Protoboard e jumpers
- Rede Wi-Fi com acesso à internet

## 📌 Mapeamento de pinos

| Componente | Pino ESP32 | Observação |
|---|---|---|
| DHT11 (dados) | **GPIO 4** | |
| MQ-135 (saída analógica) | **GPIO 34** | Pino somente de entrada, ADC de 12 bits (0–4095) |
| Sensor de chuva (digital) | **GPIO 5** | Nível `LOW` = chuva detectada |
| LED de alerta de chuva | **GPIO 18** | |
| Buzzer piezo | **GPIO 19** | |
| BMP180 (SDA / SCL) | **GPIO 21 / GPIO 22** | Pinos I²C padrão da ESP32 |

## 🏗️ Arquitetura do sistema

```
┌──────────────┐
│    DHT11     │──┐
├──────────────┤  │
│    BMP180    │──┤        ┌─────────┐   Wi-Fi / HTTP    ┌─────────────┐   ┌──────────────┐
├──────────────┤  ├──────▶ │  ESP32  │ ────────────────▶ │ ThingSpeak  │──▶│ Gráficos e   │
│   MQ-135     │──┤        └────┬────┘      (POST)        │   (nuvem)   │   │ dashboard    │
├──────────────┤  │             │                         └─────────────┘   └──────────────┘
│ Sensor chuva │──┘             ▼
└──────────────┘        LED + Buzzer (alerta local)
```

**Fluxo de funcionamento:**

1. A ESP32 conecta-se ao Wi-Fi.
2. O sensor de chuva é verificado **continuamente**; ao detectar chuva, o LED é aceso.
3. A cada **20 segundos**, os sensores (DHT11, MQ-135 e BMP180) são lidos.
4. Os dados são exibidos no **monitor serial** e enviados ao **ThingSpeak** via HTTP POST.

> O intervalo de 20 s respeita o mínimo de **15 s** entre atualizações exigido pela conta gratuita do ThingSpeak.

## 📊 Campos do ThingSpeak

| Campo | Variável | Unidade |
|---|---|---|
| `field1` | Umidade relativa | % |
| `field2` | Temperatura (DHT11) | °C |
| `field3` | Pressão relativa (ao nível do mar) | hPa |
| `field4` | Pressão absoluta | hPa |
| `field5` | Qualidade do ar (MQ-135) | PPM |
| `field6` | Status de chuva | `0` = chuva / `1` = sem chuva |

## 📚 Bibliotecas necessárias

Instale pela Arduino IDE (Gerenciador de Bibliotecas) ou pelo PlatformIO:

- `WiFi.h` e `HTTPClient.h` (já incluídas no core da ESP32)
- `Wire.h` (já incluída)
- **DHT sensor library** (Adafruit)
- **SFE_BMP180** (SparkFun)

## 🚀 Configuração e execução

1. **Clone o repositório:**
   ```bash
   git clone https://github.com/SEU-USUARIO/NOME-DO-REPOSITORIO.git
   cd NOME-DO-REPOSITORIO
   ```

2. **Crie um canal no [ThingSpeak](https://thingspeak.com)** com 6 campos (conforme a tabela acima) e copie a **Write API Key**.

3. **Configure as credenciais** no código (⚠️ nunca publique credenciais reais no GitHub):
   ```cpp
   const char* ssid = "SEU_WIFI";
   const char* pass = "SUA_SENHA";
   String apiKey   = "SUA_WRITE_API_KEY";
   ```

4. **Ajuste a altitude** do local da plantação, usada no cálculo da pressão relativa:
   ```cpp
   #define ALTITUDE 262.0   // em metros
   ```

5. **Selecione a placa** `ESP32 Dev Module`, escolha a porta COM e faça o upload.

6. **Abra o Monitor Serial** em `115200 baud` para acompanhar as leituras e o status do envio.

### 🧪 Modo de teste (dados simulados)

O código possui um bloco de **dados mock** para testar a comunicação com o ThingSpeak sem os sensores conectados. Para usar os sensores reais, **remova ou comente** o bloco marcado como