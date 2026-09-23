S2-CP02 — Projeto Motiva | Atualização Remota de Firmware (OTA)

Integrantes

|Integrante       |RM    |
|-----------------|-----:|
|Julia Aparicio   |563623|
|Gabrielly Lorentz|565806|
|Giovana Praieiro |565681|
|Heitor Barbosa   |563078|
|Maria Eduarda    |565386|
|Nicole Calasans  |564381|

────────

1. Sobre o projeto

Este projeto simula um nó IoT do Projeto Motiva utilizando um ESP32 no Wokwi para monitoramento da altura da vegetação.

As leituras são simuladas por valores pseudoaleatórios entre 10 e 20 cm. Cada sessão possui 5 leituras, realizadas com intervalo de 2 segundos, e uma nova sessão é iniciada a cada 48 segundos, contados a partir do início da sessão anterior.

A solução também implementa a atualização remota de firmware (OTA), permitindo que o ESP32 consulte uma versão disponível em um repositório remoto e obtenha uma nova versão do firmware.

────────

2. Arquitetura da solução

                    INTERNET
                       │
                       ▼
              ┌─────────────────┐
              │ Repositório     │
              │ remoto          │
              │                 │
              │ version.json    │
              │ firmware_v2.bin │
              └────────┬────────┘
                       │
                    HTTP/HTTPS
                       │
                       ▼
              ┌─────────────────┐
              │ ESP32 / Wokwi   │
              │                 │
              │ Firmware 1.0    │
              └────────┬────────┘
                       │
                       │ OTA
                       ▼
              ┌─────────────────┐
              │ Firmware 2.0    │
              │                 │
              │ Média           │
              │ Ordenação       │
              │ Mediana         │
              │ Histerese       │
              │ LED RGB         │
              └─────────────────┘

O fluxo da solução é:

Firmware 1.0 → 3 sessões → consulta version.json → identifica a versão 2.0 → obtém firmware_v2.bin → atualização OTA → reinicialização → Firmware 2.0.

────────

3. Firmware 1.0

O Firmware 1.0 realiza o monitoramento inicial da vegetação e a verificação de novas versões do firmware.

Funcionalidades

• Conexão com a rede Wokwi-GUEST;
• geração de valores pseudoaleatórios entre 10 e 20 cm;
• 5 leituras por sessão;
• intervalo de 2 segundos entre leituras;
• armazenamento das leituras em um vetor;
• cálculo da média das 5 leituras;
• nova sessão a cada 48 segundos;
• indicação do Firmware 1.0 por LED azul;
• consulta ao manifesto remoto para verificar atualizações;
• download e atualização do firmware por OTA.

────────

4. Firmware 2.0

O Firmware 2.0 mantém as funcionalidades do Firmware 1.0 e acrescenta o tratamento de dados.

Funcionalidades

• 5 leituras por sessão;
• intervalo de 2 segundos;
• cálculo da média;
• cópia e ordenação das leituras em ordem crescente;
• exibição das leituras originais;
• exibição das leituras ordenadas;
• cálculo da mediana;
• aplicação de histerese;
• indicação do estado por LED RGB.

Mediana

Como são realizadas 5 leituras, a mediana corresponde ao terceiro valor após a ordenação.

Exemplo:

Leituras originais:
18 | 12 | 15 | 14 | 20

Leituras ordenadas:
12 | 14 | 15 | 18 | 20

Mediana:
15 cm

Histerese

A decisão do estado utiliza a mediana da sessão:

|Mediana              |Estado                  |
|---------------------|------------------------|
|`>= 16 cm`           |ALERTA                  |
|`> 14 cm` e `< 16 cm`|Mantém o estado anterior|
|`<= 14 cm`           |NORMAL                  |

LED RGB

|Situação             |LED     |
|---------------------|--------|
|Firmware 1.0         |Azul    |
|Firmware 2.0 — NORMAL|Verde   |
|Firmware 2.0 — ALERTA|Vermelho|

────────

5. Atualização OTA

A atualização remota utiliza o arquivo version.json, hospedado no repositório remoto.

version.json

{
    "version": "2.0",
    "url": "https://raw.githubusercontent.com/gipraieiro/projeto-motiva-ota/main/ota/firmware_v2.bin"
}

O Firmware 1.0 consulta o manifesto após três sessões completas. A versão instalada é comparada com a versão disponível.

Durante o teste, o ESP32 identificou:

[OTA] Versao instalada: 1.0 | Versao disponivel: 2.0
[OTA] Nova versao encontrada!
[OTA] Iniciando processo de atualizacao...

Em seguida, o firmware remoto foi localizado:

[OTA] Tamanho do firmware: 274784 bytes

O arquivo utilizado para a atualização é o firmware_v2.bin.

────────

6. Repositório remoto

GitHub:

https://github.com/gipraieiro/projeto-motiva-ota

Manifesto:

https://raw.githubusercontent.com/gipraieiro/projeto-motiva-ota/main/ota/version.json

Firmware 2.0:

https://raw.githubusercontent.com/gipraieiro/projeto-motiva-ota/main/ota/firmware_v2.bin

────────

7. Estrutura do projeto

projeto-motiva-ota/
│
├── README.md
├── sketch.ino
├── diagram.json
├── libraries.txt
├── wokwi-project.txt
│
└── ota/
    ├── version.json
    └── firmware_v2.bin

────────

8. Instruções de execução

1. Abrir o projeto no Wokwi.
2. Iniciar a simulação do ESP32.
3. Abrir o Serial Monitor.
4. Verificar a inicialização do Firmware 1.0.
5. Acompanhar as 5 leituras realizadas a cada 2 segundos.
6. Verificar o cálculo da média.
7. Aguardar três sessões completas.
8. Acompanhar a consulta ao version.json.
9. Verificar a identificação da versão 2.0.
10. Acompanhar o processo de atualização OTA.
11. Após a reinicialização, verificar a execução do Firmware 2.0.
12. Verificar as leituras originais, leituras ordenadas, média, mediana, estado e indicação do LED.

Link público do Wokwi

COLE AQUI O LINK DO PROJETO WOKWI

────────

Link público do Wokwi: https://wokwi.com/projects/475788954877564929

9. Tratamento de situações

O sistema informa no Serial Monitor situações como:

• falha de conexão Wi-Fi;
• falha de acesso ao manifesto;
• manifesto sem versão ou URL;
• erro na interpretação do JSON;
• firmware já atualizado;
• falha no download do firmware;
• firmware maior que a partição disponível;
• ausência de partição OTA;
• erro durante a atualização.
