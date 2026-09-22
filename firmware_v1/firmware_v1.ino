// ============================================================
// PROJETO MOTIVA - FIRMWARE 1.0
// Monitoramento de vegetação - ESP32
// ============================================================

// ---------- Pinos do LED RGB ----------
const int LED_R = 25;
const int LED_G = 26;
const int LED_B = 27;

// ---------- Configurações ----------
const int TOTAL_LEITURAS = 5;
const unsigned long INTERVALO_LEITURA = 2000;  // 2 segundos
const unsigned long DURACAO_SESSAO = 48000;   // 48 segundos

// ---------- Dados da sessão ----------
int leituras[TOTAL_LEITURAS];
int quantidadeLeituras = 0;

// ---------- Controle de tempo ----------
unsigned long inicioSessao = 0;
unsigned long ultimaLeitura = 0;

// ============================================================
// LED AZUL - Firmware 1.0
// ============================================================

void ledAzul() {
  digitalWrite(LED_R, LOW);
  digitalWrite(LED_G, LOW);
  digitalWrite(LED_B, HIGH);
}

// ============================================================
// Geração da leitura simulada
// Valores entre 10 e 20 cm
// ============================================================

int gerarLeitura() {
  return random(10, 21);
}

// ============================================================
// Processamento da sessão
// ============================================================

void processarSessao() {

  Serial.println();
  Serial.println("===== FIM DA SESSAO =====");

  Serial.print("Leituras: ");

  int soma = 0;

  for (int i = 0; i < TOTAL_LEITURAS; i++) {

    Serial.print(leituras[i]);

    if (i < TOTAL_LEITURAS - 1) {
      Serial.print(" | ");
    }

    soma += leituras[i];
  }

  float media = (float)soma / TOTAL_LEITURAS;

  Serial.println();

  Serial.print("Media da vegetacao: ");
  Serial.print(media, 2);
  Serial.println(" cm");

  Serial.println("==========================");
  Serial.println();
}

// ============================================================
// Inicia uma nova sessão
// ============================================================

void iniciarNovaSessao() {

  quantidadeLeituras = 0;

  // O início da sessão é exatamente neste momento
  inicioSessao = millis();

  // IMPORTANTE:
  // Colocamos ultimaLeitura como início - 2 segundos
  // para que a primeira leitura aconteça imediatamente.
  ultimaLeitura = inicioSessao - INTERVALO_LEITURA;

  Serial.println();
  Serial.println("===== NOVA SESSAO =====");
  Serial.println("Iniciando monitoramento...");
}

// ============================================================
// SETUP
// ============================================================

void setup() {

  Serial.begin(115200);

  pinMode(LED_R, OUTPUT);
  pinMode(LED_G, OUTPUT);
  pinMode(LED_B, OUTPUT);

  // Inicializa o gerador de números aleatórios
  randomSeed(micros());

  Serial.println();
  Serial.println("================================");
  Serial.println("       PROJETO MOTIVA");
  Serial.println("       FIRMWARE 1.0");
  Serial.println("================================");

  // FW1 utiliza LED azul
  ledAzul();

  Serial.println("Firmware 1.0 iniciado.");
  Serial.println("LED: AZUL");
  Serial.println("Leituras: 5");
  Serial.println("Intervalo: 2 segundos");
  Serial.println("Sessao: 48 segundos");
  Serial.println();

  // Inicia a primeira sessão
  iniciarNovaSessao();
}

// ============================================================
// LOOP
// ============================================================

void loop() {

  unsigned long agora = millis();

  // ----------------------------------------------------------
  // COLETA DAS 5 LEITURAS
  // ----------------------------------------------------------

  if (quantidadeLeituras < TOTAL_LEITURAS) {

    if (agora - ultimaLeitura >= INTERVALO_LEITURA) {

      int leitura = gerarLeitura();

      leituras[quantidadeLeituras] = leitura;

      quantidadeLeituras++;

      Serial.print("Leitura ");
      Serial.print(quantidadeLeituras);
      Serial.print(": ");
      Serial.print(leitura);
      Serial.println(" cm");

      ultimaLeitura = agora;

      // Após a quinta leitura, calcula a média
      if (quantidadeLeituras == TOTAL_LEITURAS) {
        processarSessao();
      }
    }

    return;
  }

  // ----------------------------------------------------------
  // NOVA SESSÃO A CADA 48 SEGUNDOS
  // CONTADOS DESDE O INÍCIO DA SESSÃO ANTERIOR
  // ----------------------------------------------------------

  if (agora - inicioSessao >= DURACAO_SESSAO) {

    iniciarNovaSessao();
  }
}