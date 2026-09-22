// ============================================================
// PROJETO MOTIVA - FIRMWARE 1.0
// Monitoramento de vegetação + Atualização OTA
// ESP32 - Wokwi
// ============================================================

#include <WiFi.h>
#include <HTTPClient.h>
#include <Update.h>
#include <ArduinoJson.h>

// ============================================================
// CONFIGURAÇÕES DE WI-FI
// ============================================================

const char *WIFI_SSID = "Wokwi-GUEST";
const char *WIFI_PASS = "";

// ============================================================
// CONFIGURAÇÕES OTA
// ============================================================

const char *CURRENT_VERSION = "1.0";

const char *MANIFEST_URL =
    "https://raw.githubusercontent.com/gipraieiro/projeto-motiva-ota/main/ota/version.json";

// Contador de sessões completas
int contadorSessoes = 0;

// ============================================================
// PINOS DO LED RGB
// ============================================================

const int LED_R = 25;
const int LED_G = 26;
const int LED_B = 27;

// ============================================================
// CONFIGURAÇÕES DO MONITORAMENTO
// ============================================================

const int TOTAL_LEITURAS = 5;

const unsigned long INTERVALO_LEITURA = 2000; // 2 segundos

const unsigned long DURACAO_SESSAO = 48000; // 48 segundos

// ============================================================
// DADOS DA SESSÃO
// ============================================================

int leituras[TOTAL_LEITURAS];

int quantidadeLeituras = 0;

// ============================================================
// CONTROLE DE TEMPO
// ============================================================

// Momento de início da sessão atual
unsigned long inicioSessao = 0;

// Momento da última leitura
unsigned long ultimaLeitura = 0;

// Momento exato do início da próxima sessão
unsigned long proximaSessao = 0;

// ============================================================
// LED AZUL - FIRMWARE 1.0
// ============================================================

void ledAzul()
{

  digitalWrite(LED_R, LOW);
  digitalWrite(LED_G, LOW);
  digitalWrite(LED_B, HIGH);
}

// ============================================================
// CONEXÃO WI-FI
// ============================================================

void conectarWiFi()
{

  Serial.print("[Wi-Fi] Conectando a ");
  Serial.println(WIFI_SSID);

  WiFi.begin(WIFI_SSID, WIFI_PASS);

  int tentativas = 0;

  while (
      WiFi.status() != WL_CONNECTED &&
      tentativas < 20)
  {

    delay(500);

    Serial.print(".");

    tentativas++;
  }

  if (WiFi.status() == WL_CONNECTED)
  {

    Serial.println();
    Serial.println("[Wi-Fi] Conectado com sucesso!");

    Serial.print("[Wi-Fi] IP: ");
    Serial.println(WiFi.localIP());
  }
  else
  {

    Serial.println();
    Serial.println("[Wi-Fi] Erro ao conectar ao Wi-Fi.");
  }
}

// ============================================================
// DOWNLOAD E GRAVAÇÃO DO FIRMWARE VIA OTA
// ============================================================

void performOTA(String binUrl)
{

  Serial.println();
  Serial.println("[OTA] Iniciando download do firmware...");

  Serial.print("[OTA] URL: ");
  Serial.println(binUrl);

  HTTPClient http;

  http.begin(binUrl);

  int httpCode = http.GET();

  // ----------------------------------------------------------
  // DOWNLOAD REALIZADO
  // ----------------------------------------------------------

  if (httpCode == HTTP_CODE_OK)
  {

    int contentLength = http.getSize();

    Serial.print("[OTA] Tamanho do firmware: ");
    Serial.print(contentLength);
    Serial.println(" bytes");

    // --------------------------------------------------------
    // PREPARA A MEMÓRIA PARA O NOVO FIRMWARE
    // --------------------------------------------------------

    bool canBegin = Update.begin(contentLength);

    if (canBegin)
    {

      Serial.println(
          "[OTA] Gravando nova versão no ESP32...");

      WiFiClient *client =
          http.getStreamPtr();

      // ------------------------------------------------------
      // GRAVA O FIRMWARE
      // ------------------------------------------------------

      size_t written =
          Update.writeStream(*client);

      if (written == contentLength)
      {

        Serial.println(
            "[OTA] Gravação concluída com sucesso!");
      }
      else
      {

        Serial.printf(
            "[OTA] Erro: escreveu %d de %d bytes\n",
            written,
            contentLength);
      }

      // ------------------------------------------------------
      // FINALIZA ATUALIZAÇÃO
      // ------------------------------------------------------

      if (Update.end())
      {

        if (Update.isFinished())
        {

          Serial.println(
              "[OTA] Atualização finalizada com sucesso!");

          Serial.println(
              "[OTA] Reiniciando ESP32...");

          delay(2000);

          ESP.restart();
        }
        else
        {

          Serial.println(
              "[OTA] Erro: atualização não foi finalizada.");
        }
      }
      else
      {

        Serial.printf(
            "[OTA] Erro no Update.end(): %d\n",
            Update.getError());
      }
    }
    else
    {

      Serial.println(
          "[OTA] Erro: espaço insuficiente na flash para atualização.");
    }
  }
  else
  {

    Serial.printf(
        "[OTA] Erro ao baixar o arquivo .bin. HTTP Code: %d\n",
        httpCode);
  }

  http.end();
}

// ============================================================
// CONSULTA DO MANIFESTO E COMPARAÇÃO DE VERSÃO
// ============================================================

void checkAndPerformOTA()
{

  // ----------------------------------------------------------
  // VERIFICA WI-FI
  // ----------------------------------------------------------

  if (WiFi.status() != WL_CONNECTED)
  {

    Serial.println(
        "[OTA] Falha: sem conexão com a Internet.");

    return;
  }

  Serial.println();
  Serial.println(
      "[OTA] Verificando se há atualizações disponíveis...");

  Serial.print("[OTA] Manifesto: ");
  Serial.println(MANIFEST_URL);

  HTTPClient http;

  http.begin(MANIFEST_URL);

  int httpCode = http.GET();

  // ----------------------------------------------------------
  // MANIFESTO ACESSÍVEL
  // ----------------------------------------------------------

  if (httpCode == HTTP_CODE_OK)
  {

    String payload = http.getString();

    Serial.println(
        "[OTA] Manifesto recebido.");

    // Mostra exatamente o que o servidor enviou
    // para facilitar o diagnóstico do JSON.
    Serial.println(
        "[OTA] Conteudo recebido:");

    Serial.println(payload);

    // --------------------------------------------------------
    // INTERPRETA JSON
    // --------------------------------------------------------

    StaticJsonDocument<512> doc;

    DeserializationError error =
        deserializeJson(doc, payload);

    // --------------------------------------------------------
    // JSON VÁLIDO
    // --------------------------------------------------------

    if (!error)
    {

      const char *serverVersion =
          doc["version"];

      const char *downloadUrl =
          doc["url"];

      // ------------------------------------------------------
      // VERIFICA CAMPOS DO JSON
      // ------------------------------------------------------

      if (
          serverVersion == nullptr ||
          downloadUrl == nullptr)
      {

        Serial.println(
            "[OTA] Erro: manifesto sem versão ou URL do firmware.");

        http.end();

        return;
      }

      Serial.printf(
          "[OTA] Versao instalada: %s | Versao disponivel: %s\n",
          CURRENT_VERSION,
          serverVersion);

      // ------------------------------------------------------
      // COMPARAÇÃO DE VERSÃO
      // ------------------------------------------------------

      if (
          strcmp(
              serverVersion,
              CURRENT_VERSION) > 0)
      {

        Serial.println(
            "[OTA] Nova versao encontrada!");

        Serial.println(
            "[OTA] Iniciando processo de atualizacao...");

        performOTA(
            String(downloadUrl));
      }
      else
      {

        Serial.println(
            "[OTA] O firmware ja esta na versao mais recente.");
      }
    }

    // --------------------------------------------------------
    // JSON INVÁLIDO
    // --------------------------------------------------------

    else
    {

      Serial.print(
          "[OTA] Erro ao interpretar JSON: ");

      Serial.println(
          error.c_str());
    }
  }

  // ----------------------------------------------------------
  // ERRO HTTP
  // ----------------------------------------------------------

  else
  {

    Serial.printf(
        "[OTA] Erro ao acessar o manifesto version.json. HTTP: %d\n",
        httpCode);
  }

  http.end();
}

// ============================================================
// GERAÇÃO DA LEITURA SIMULADA
// Valores entre 10 e 20 cm
// ============================================================

int gerarLeitura()
{

  return random(10, 21);
}

// ============================================================
// PROCESSAMENTO DA SESSÃO
// ============================================================

void processarSessao()
{

  Serial.println();

  Serial.println(
      "===== FIM DA SESSAO =====");

  Serial.print(
      "Leituras: ");

  int soma = 0;

  for (
      int i = 0;
      i < TOTAL_LEITURAS;
      i++)
  {

    Serial.print(
        leituras[i]);

    if (
        i < TOTAL_LEITURAS - 1)
    {

      Serial.print(
          " | ");
    }

    soma += leituras[i];
  }

  // ----------------------------------------------------------
  // CALCULA MÉDIA
  // ----------------------------------------------------------

  float media =
      (float)soma / TOTAL_LEITURAS;

  Serial.println();

  Serial.print(
      "Media da vegetacao: ");

  Serial.print(
      media,
      2);

  Serial.println(
      " cm");

  Serial.println(
      "==========================");

  Serial.println();

  // ----------------------------------------------------------
  // INCREMENTA CONTADOR DE SESSÕES
  // ----------------------------------------------------------

  contadorSessoes++;

  // Até a terceira sessão mostra X/3.
  // Depois disso mostra somente o número da sessão.
  if (contadorSessoes <= 3)
  {

    Serial.printf(
        "Sessoes concluidas: %d/3\n",
        contadorSessoes);
  }
  else
  {

    Serial.printf(
        "Sessoes concluidas: %d\n",
        contadorSessoes);
  }
}

// ============================================================
// INICIA UMA NOVA SESSÃO
// ============================================================

void iniciarNovaSessao()
{

  // Zera quantidade de leituras
  quantidadeLeituras = 0;

  // ----------------------------------------------------------
  // PRIMEIRA SESSÃO
  // ----------------------------------------------------------

  if (inicioSessao == 0)
  {

    inicioSessao =
        millis();

    proximaSessao =
        inicioSessao +
        DURACAO_SESSAO;
  }

  // ----------------------------------------------------------
  // SESSÕES SEGUINTES
  // ----------------------------------------------------------

  else
  {

    inicioSessao =
        proximaSessao;

    proximaSessao +=
        DURACAO_SESSAO;
  }

  // ----------------------------------------------------------
  // PRIMEIRA LEITURA IMEDIATAMENTE
  // ----------------------------------------------------------

  ultimaLeitura =
      inicioSessao -
      INTERVALO_LEITURA;

  Serial.println();

  Serial.println(
      "===== NOVA SESSAO =====");

  Serial.print(
      "Inicio da sessao: ");

  Serial.print(
      inicioSessao);

  Serial.println(
      " ms");

  Serial.println(
      "Iniciando monitoramento...");
}

// ============================================================
// SETUP
// ============================================================

void setup()
{

  Serial.begin(115200);

  // ----------------------------------------------------------
  // CONFIGURAÇÃO DOS PINOS
  // ----------------------------------------------------------

  pinMode(
      LED_R,
      OUTPUT);

  pinMode(
      LED_G,
      OUTPUT);

  pinMode(
      LED_B,
      OUTPUT);

  // ----------------------------------------------------------
  // GERADOR DE NÚMEROS ALEATÓRIOS
  // ----------------------------------------------------------

  randomSeed(
      micros());

  // ----------------------------------------------------------
  // IDENTIFICAÇÃO DO FIRMWARE
  // ----------------------------------------------------------

  Serial.println();

  Serial.println(
      "================================");

  Serial.println(
      "        PROJETO MOTIVA");

  Serial.println(
      "        FIRMWARE 1.0");

  Serial.println(
      "================================");

  // ----------------------------------------------------------
  // FW1 UTILIZA LED AZUL
  // ----------------------------------------------------------

  ledAzul();

  Serial.println(
      "Firmware 1.0 iniciado.");

  Serial.println(
      "LED: AZUL");

  Serial.println(
      "Leituras: 5");

  Serial.println(
      "Intervalo: 2 segundos");

  Serial.println(
      "Sessao: 48 segundos");

  Serial.println();

  // ----------------------------------------------------------
  // CONEXÃO WI-FI
  // ----------------------------------------------------------

  conectarWiFi();

  // ----------------------------------------------------------
  // INICIA PRIMEIRA SESSÃO
  // ----------------------------------------------------------

  iniciarNovaSessao();
}

// ============================================================
// LOOP PRINCIPAL
// ============================================================

void loop()
{

  unsigned long agora =
      millis();

  // ==========================================================
  // COLETA DAS 5 LEITURAS
  // ==========================================================

  if (
      quantidadeLeituras <
      TOTAL_LEITURAS)
  {

    if (
        agora - ultimaLeitura >=
        INTERVALO_LEITURA)
    {

      // ------------------------------------------------------
      // GERA LEITURA
      // ------------------------------------------------------

      int leitura =
          gerarLeitura();

      // ------------------------------------------------------
      // ARMAZENA LEITURA
      // ------------------------------------------------------

      leituras[quantidadeLeituras] = leitura;

      quantidadeLeituras++;

      // ------------------------------------------------------
      // MOSTRA LEITURA
      // ------------------------------------------------------

      Serial.print(
          "Leitura ");

      Serial.print(
          quantidadeLeituras);

      Serial.print(
          ": ");

      Serial.print(
          leitura);

      Serial.println(
          " cm");

      ultimaLeitura =
          agora;

      // ------------------------------------------------------
      // QUINTA LEITURA
      // ------------------------------------------------------

      if (
          quantidadeLeituras ==
          TOTAL_LEITURAS)
      {

        processarSessao();
      }
    }

    return;
  }

  // ==========================================================
  // AGUARDA O PRÓXIMO MARCO DE 48 SEGUNDOS
  // ==========================================================

  if (
      agora >= proximaSessao)
  {

    // --------------------------------------------------------
    // APÓS 3 SESSÕES COMPLETAS,
    // VERIFICA ATUALIZAÇÃO OTA
    // --------------------------------------------------------

    if (
        contadorSessoes >= 3)
    {

      Serial.println();

      Serial.println(
          "[OTA] 3 sessoes completas.");

      Serial.println(
          "[OTA] Iniciando verificacao antes da proxima sessao.");

      checkAndPerformOTA();
    }

    // --------------------------------------------------------
    // INICIA NOVA SESSÃO
    // --------------------------------------------------------

    iniciarNovaSessao();
  }
}