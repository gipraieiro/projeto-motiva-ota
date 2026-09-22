/*
  ============================================================
  PROJETO MOTIVA - Firmware 2.0
  ============================================================
  ESP32

  Funcionamento:
  - Realiza 5 leituras por sessão
  - Intervalo de 2 segundos entre leituras
  - Calcula média e mediana
  - Ordena uma cópia das leituras
  - Utiliza histerese para definir o estado
  - NORMAL  -> LED verde
  - ALERTA  -> LED vermelho
  - Nova sessão a cada 48 segundos
  ============================================================
*/

// ============================================================
// CONFIGURAÇÕES GERAIS
// ============================================================

const int NUM_LEITURAS = 5;

const unsigned long INTERVALO_LEITURA = 2000UL;   // 2 segundos
const unsigned long INTERVALO_SESSAO  = 48000UL;  // 48 segundos


// ============================================================
// VALORES SIMULADOS
// ============================================================

const int VALOR_MIN = 10;
const int VALOR_MAX = 20;


// ============================================================
// LIMITES DA HISTERese
// ============================================================

const float LIMITE_ALERTA = 16.0;
const float LIMITE_NORMAL = 14.0;

// Entre 14 e 16:
// mantém o estado anterior


// ============================================================
// PINOS DO LED RGB
// ============================================================

const int PIN_LED_VERMELHO = 25;
const int PIN_LED_VERDE    = 26;
const int PIN_LED_AZUL     = 27;


// ============================================================
// ESTADOS
// ============================================================

enum Estado {
  NORMAL,
  ALERTA
};

Estado estadoAtual = NORMAL;


// ============================================================
// VARIÁVEIS DA SESSÃO
// ============================================================

float leituras[NUM_LEITURAS];

int indiceLeituraAtual = 0;

unsigned long tInicioSessao = 0;
unsigned long tUltimaLeitura = 0;

bool sessaoEmColeta = true;

int numeroSessao = 0;


// ============================================================
// SETUP
// ============================================================

void setup() {

  Serial.begin(115200);

  // Configuração dos pinos do RGB
  pinMode(PIN_LED_VERMELHO, OUTPUT);
  pinMode(PIN_LED_VERDE, OUTPUT);
  pinMode(PIN_LED_AZUL, OUTPUT);

  // Inicializa o gerador de números aleatórios
  randomSeed(micros());

  // ========================================================
  // IDENTIFICAÇÃO DA VERSÃO
  // ========================================================

  Serial.println();
  Serial.println("========================================");
  Serial.println("        PROJETO MOTIVA");
  Serial.println("        FIRMWARE 2.0");
  Serial.println("========================================");

  Serial.println("Firmware 2.0 iniciado.");

  // Estado inicial = NORMAL
  estadoAtual = NORMAL;

  atualizarLED();

  Serial.println("Estado inicial: NORMAL");
  Serial.println("LED: VERDE");
  Serial.println("Leituras: 5");
  Serial.println("Intervalo: 2 segundos");
  Serial.println("Sessao: 48 segundos");
  Serial.println();

  // ========================================================
  // INICIA A PRIMEIRA SESSÃO
  // ========================================================

  tInicioSessao = millis();

  // Permite a primeira leitura imediatamente
  tUltimaLeitura = tInicioSessao - INTERVALO_LEITURA;

  indiceLeituraAtual = 0;

  sessaoEmColeta = true;

  numeroSessao++;

  Serial.print("===== NOVA SESSAO ");
  Serial.print(numeroSessao);
  Serial.println(" =====");
}


// ============================================================
// LOOP PRINCIPAL
// ============================================================

void loop() {

  unsigned long agora = millis();


  // ========================================================
  // FASE 1 - COLETA DAS 5 LEITURAS
  // ========================================================

  if (sessaoEmColeta) {

    if (indiceLeituraAtual < NUM_LEITURAS) {

      // Verifica se já passou o intervalo de 2 segundos
      if (agora - tUltimaLeitura >= INTERVALO_LEITURA) {

        tUltimaLeitura = agora;

        realizarLeitura();
      }
    }

    else {

      // As 5 leituras foram coletadas
      processarSessao();

      sessaoEmColeta = false;
    }
  }


  // ========================================================
  // FASE 2 - AGUARDA OS 48 SEGUNDOS
  // ========================================================

  if (!sessaoEmColeta) {

    // Os 48 segundos são contados desde o início
    // da sessão atual
    if (agora - tInicioSessao >= INTERVALO_SESSAO) {

      iniciarNovaSessao(agora);
    }
  }
}


// ============================================================
// REALIZA LEITURA
// ============================================================

void realizarLeitura() {

  // Gera valor aleatório entre 10 e 20
  float valor = random(VALOR_MIN, VALOR_MAX + 1);

  // Armazena a leitura
  leituras[indiceLeituraAtual] = valor;

  // Exibe no Serial Monitor
  Serial.print("Leitura ");
  Serial.print(indiceLeituraAtual + 1);
  Serial.print(": ");
  Serial.print(valor, 0);
  Serial.println(" cm");

  // Avança para próxima leitura
  indiceLeituraAtual++;
}


// ============================================================
// PROCESSA A SESSÃO
// ============================================================

void processarSessao() {

  Serial.println();
  Serial.println("===== PROCESSAMENTO DA SESSAO =====");


  // ========================================================
  // CÓPIA DAS LEITURAS
  // ========================================================

  float ordenado[NUM_LEITURAS];

  for (int i = 0; i < NUM_LEITURAS; i++) {

    ordenado[i] = leituras[i];
  }


  // ========================================================
  // ORDENAÇÃO - BUBBLE SORT
  // ========================================================

  for (int i = 0; i < NUM_LEITURAS - 1; i++) {

    for (int j = 0; j < NUM_LEITURAS - 1 - i; j++) {

      if (ordenado[j] > ordenado[j + 1]) {

        float temp = ordenado[j];

        ordenado[j] = ordenado[j + 1];

        ordenado[j + 1] = temp;
      }
    }
  }


  // ========================================================
  // LEITURAS ORIGINAIS
  // ========================================================

  Serial.print("Leituras originais: ");

  for (int i = 0; i < NUM_LEITURAS; i++) {

    Serial.print(leituras[i], 0);

    if (i < NUM_LEITURAS - 1) {
      Serial.print(" | ");
    }
  }

  Serial.println();


  // ========================================================
  // LEITURAS ORDENADAS
  // ========================================================

  Serial.print("Leituras ordenadas: ");

  for (int i = 0; i < NUM_LEITURAS; i++) {

    Serial.print(ordenado[i], 0);

    if (i < NUM_LEITURAS - 1) {
      Serial.print(" | ");
    }
  }

  Serial.println();


  // ========================================================
  // CALCULA MÉDIA
  // ========================================================

  float soma = 0;

  for (int i = 0; i < NUM_LEITURAS; i++) {

    soma += leituras[i];
  }

  float media = soma / NUM_LEITURAS;


  // ========================================================
  // CALCULA MEDIANA
  // ========================================================

  // Como temos 5 valores, a mediana é o terceiro
  // elemento após a ordenação.
  float mediana = ordenado[2];


  // ========================================================
  // EXIBE MÉDIA
  // ========================================================

  Serial.print("Media da sessao: ");
  Serial.print(media, 1);
  Serial.println(" cm");


  // ========================================================
  // EXIBE MEDIANA
  // ========================================================

  Serial.print("Mediana da sessao: ");
  Serial.print(mediana, 1);
  Serial.println(" cm");


  // ========================================================
  // HISTERese
  // ========================================================

  Estado estadoAnterior = estadoAtual;


  // Mediana >= 16
  // ALERTA
  if (mediana >= LIMITE_ALERTA) {

    estadoAtual = ALERTA;
  }


  // Mediana <= 14
  // NORMAL
  else if (mediana <= LIMITE_NORMAL) {

    estadoAtual = NORMAL;
  }


  // Entre 14 e 16:
  // mantém estado anterior


  // ========================================================
  // EXIBE ESTADO
  // ========================================================

  Serial.print("Estado: ");

  if (estadoAtual == ALERTA) {

    Serial.print("ALERTA");

  } else {

    Serial.print("NORMAL");
  }


  // Verifica se houve alteração
  if (estadoAtual == estadoAnterior) {

    Serial.println(" (mantido)");

  } else {

    Serial.println(" (alterado)");
  }


  // ========================================================
  // ATUALIZA LED
  // ========================================================

  atualizarLED();


  Serial.println("==================================");
  Serial.println("Proxima sessao em 48 segundos.");
  Serial.println();
}


// ============================================================
// INICIA NOVA SESSÃO
// ============================================================

void iniciarNovaSessao(unsigned long agora) {

  // Marca o início da nova sessão
  tInicioSessao = agora;

  // Permite primeira leitura imediatamente
  tUltimaLeitura = agora - INTERVALO_LEITURA;

  // Reinicia índice
  indiceLeituraAtual = 0;

  // Volta para coleta
  sessaoEmColeta = true;

  // Incrementa número da sessão
  numeroSessao++;


  Serial.println();
  Serial.print("===== NOVA SESSAO ");
  Serial.print(numeroSessao);
  Serial.println(" =====");
  Serial.println("Iniciando monitoramento...");
}


// ============================================================
// ATUALIZA LED RGB
// ============================================================
//
// NORMAL -> Verde
// ALERTA -> Vermelho
//
// Azul permanece desligado na FW2.
// ============================================================

void atualizarLED() {

  // Primeiro desliga as três cores
  digitalWrite(PIN_LED_VERMELHO, LOW);
  digitalWrite(PIN_LED_VERDE, LOW);
  digitalWrite(PIN_LED_AZUL, LOW);


  if (estadoAtual == ALERTA) {

    // ALERTA = vermelho
    digitalWrite(PIN_LED_VERMELHO, HIGH);

  } else {

    // NORMAL = verde
    digitalWrite(PIN_LED_VERDE, HIGH);
  }
}