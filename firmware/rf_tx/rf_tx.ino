/*
 * ============================================================
 *  Transmissor do controle do ventilador (ESP32 + CC1101)
 * ============================================================
 *
 *  O QUE FAZ
 *  Imita o controle remoto do ventilador: monta o quadro de 32
 *  bits (protocolo que descobrimos) e transmite pelo CC1101.
 *
 *  COMO USAR
 *  - Botao no D22: manda "Luz ON/OFF".
 *  - Serial Monitor (115200, "New Line"): digite um comando,
 *    ex: luz, off, v1 ... v6, fanoff. Digite "ajuda" para a lista.
 *
 *  PROTOCOLO DO CONTROLE (descoberto no gag_cc1101_receive)
 *    byte 1-2: ID do controle (0x41 0x19)
 *    byte 3:   3 bits fixos (111) + comando (5 bits)
 *    byte 4:   contador (3 bits) + verificacao (4 bits) + 0
 *    verif = (11 - ((cmd >> 1) & 7) - contador
 *             + 8 * (bit0 XOR bit4 do cmd)) mod 16
 *
 *  FORMA DO SINAL (OOK, 433,92 MHz)
 *    cabecalho: ligado 5,7 ms, desligado 0,76 ms
 *    bit 0: ligado 256 us, desligado 760 us
 *    bit 1: ligado 760 us, desligado 256 us
 *    ultimo bit termina com uma pausa de ~2,6 ms
 *    o quadro se repete varias vezes a cada "clique"
 *
 *  LIGACAO: igual ao gag_cc1101_receive
 *    GDO0 -> D4, CSN -> D5, SCK -> D18, MOSI -> D23, MISO -> D19
 *    Botao -> D22 e GND
 */

#include <SPI.h>
#include <cc1101.h>

using namespace CC1101;


// ============================================================
//  PINOS
// ============================================================
#define PIN_SCK    18
#define PIN_MISO   19
#define PIN_MOSI   23
#define PIN_CSN     5
#define PIN_GDO0    4
#define PIN_BOTAO  22


// ============================================================
//  CONTROLE A IMITAR (mude aqui para outro controle do mesmo tipo)
// ============================================================
const uint16_t ID_CONTROLE = 0x4119;  // bytes 1 e 2
const uint8_t  ID_EXTRA    = 0x07;    // 3 bits fixos do byte 3 (111)


// ============================================================
//  TEMPOS DO SINAL (microssegundos)
// ============================================================
// Medido pelo Broadlink no controle de verdade (mais fiel que o
// CC1101, cujo ganho automatico inventava um "cabecalho" de 5,7 ms
// durante o silencio entre quadros).
const int T_CURTO       = 260;
const int T_LONGO       = 790;
const int T_PAUSA_QUADRO = 8500;  // silencio entre um quadro e o proximo
const int PULSOS_PREAMBULO = 4;   // pulsos curtos no inicio do clique
const int T_PAUSA_PREAMBULO = 9300;
const int REPETICOES    = 8;      // quantas vezes o quadro e enviado por clique


// ============================================================
//  COMANDOS (nome digitado no Serial -> codigo de 5 bits)
// ============================================================
struct Comando {
  const char* nome;
  uint8_t     codigo;
  const char* descricao;
};

Comando comandos[] = {
  { "luz",     8,  "Luz ON/OFF" },
  { "off",     6,  "Desliga tudo" },
  { "fanoff", 22,  "Desliga ventilador" },
  { "v1",     16,  "Velocidade 1" },
  { "v2",     18,  "Velocidade 2" },
  { "v3",     28,  "Velocidade 3" },
  { "v4",     10,  "Velocidade 4" },
  { "v5",     15,  "Velocidade 5" },
  { "v6",     12,  "Velocidade 6" },
  { "vento",  21,  "Vento natural" },
  { "rf",      7,  "Verao/inverno" },
  { "mudo",   31,  "Liga/desliga o bipe" },
  { "brilho+", 1,  "Brilho +" },
  { "brilho-",11,  "Brilho -" },
  { "k+",     13,  "Cor mais fria" },
  { "k-",     14,  "Cor mais quente" },
  { "k",      30,  "Troca a cor (6000/3000/4000K)" },
  { "lua",     5,  "Luz noturna" },
  { "1h",      2,  "Timer 1 hora" },
  { "2h",      9,  "Timer 2 horas" },
  { "4h",     25,  "Timer 4 horas" },
};
const int TOTAL_COMANDOS = sizeof(comandos) / sizeof(comandos[0]);

uint8_t contador = 0;   // avanca a cada envio, de 0 a 7

Radio radio(PIN_CSN, PIN_GDO0);


// ============================================================
//  MONTAGEM DO QUADRO
// ============================================================
uint8_t calcularVerificacao(uint8_t cmd, uint8_t cont) {
  int bit0 = cmd & 1;
  int bit4 = (cmd >> 4) & 1;
  int v = 11 - ((cmd >> 1) & 7) - cont + 8 * (bit0 ^ bit4);
  return v & 0x0F;   // mod 16 (funciona tambem para negativos)
}

uint32_t montarQuadro(uint8_t cmd, uint8_t cont) {
  uint32_t q = (uint32_t)ID_CONTROLE << 16;                           // bytes 1-2
  q |= (uint32_t)((ID_EXTRA << 5) | (cmd & 0x1F)) << 8;               // byte 3
  q |= ((cont & 0x07) << 5) | (calcularVerificacao(cmd, cont) << 1);  // byte 4
  return q;
}

void imprimirHex(uint32_t v) {
  for (int s = 24; s >= 0; s -= 8) {
    uint8_t b = v >> s;
    if (b < 0x10) Serial.print('0');
    Serial.print(b, HEX);
    Serial.print(' ');
  }
}

// Confere a formula contra quadros capturados do controle de verdade
void autoTeste() {
  struct { uint8_t cmd, cont; uint32_t esperado; } casos[] = {
    {  8, 0, 0x4119E80E },   // Luz, contador 0
    {  8, 6, 0x4119E8C2 },   // Luz, contador 6
    { 12, 6, 0x4119ECDE },   // Velocidade 6
    { 16, 1, 0x4119F024 },   // Velocidade 1
    { 22, 7, 0x4119F6F2 },   // FANOFF
  };
  bool ok = true;
  for (auto& c : casos) {
    if (montarQuadro(c.cmd, c.cont) != c.esperado) ok = false;
  }
  Serial.println(ok ? "Auto-teste da formula: OK" : "Auto-teste da formula: FALHOU");
}


// ============================================================
//  CC1101 EM MODO TRANSMISSAO
// ============================================================
// Escreve a tabela de potencia (PATABLE). No OOK, a posicao 0 e o
// nivel "desligado" e a posicao 1 e o nivel "ligado".
// Le um registrador de status (precisa do bit 0xC0)
uint8_t lerStatus(uint8_t endereco) {
  SPI.beginTransaction(SPISettings(1000000, MSBFIRST, SPI_MODE0));
  digitalWrite(PIN_CSN, LOW);
  while (digitalRead(PIN_MISO) == HIGH);
  SPI.transfer(endereco | 0xC0);
  uint8_t v = SPI.transfer(0x00);
  digitalWrite(PIN_CSN, HIGH);
  SPI.endTransaction();
  return v;
}

void escreverPatable(uint8_t desligado, uint8_t ligado) {
  SPI.beginTransaction(SPISettings(1000000, MSBFIRST, SPI_MODE0));
  digitalWrite(PIN_CSN, LOW);
  while (digitalRead(PIN_MISO) == HIGH);
  SPI.transfer(0x3E | 0x40);   // PATABLE, escrita em sequencia (burst)
  SPI.transfer(desligado);
  SPI.transfer(ligado);
  digitalWrite(PIN_CSN, HIGH);
  SPI.endTransaction();
}

void configurarTransmissor() {
  radio.idle();
  radio.setModulation(MOD_ASK_OOK);
  radio.setFrequency(433.92);
  radio.setDataRate(100);              // modulador amostra o GDO0 bem rapido
  radio.setPacketLengthMode(PKT_LEN_MODE_INFINITE);
  radio.setSyncMode(SYNC_MODE_NO_PREAMBLE);
  radio.setCrc(false);
  radio.setDataWhitening(false);
  radio.setManchester(false);
  radio.setFEC(false);
  radio.setPacketFormat(PKT_FORMAT_ASYNC_SERIAL);

  radio.writeReg(0x02, 0x0D);          // IOCFG0: modo serial assincrono
  radio.writeReg(0x08, 0x32);          // PKTCTRL0: serial assincrono, sem pacotes
  radio.writeReg(0x22, 0x11);          // FREND0: OOK usa PATABLE[0] e [1]
  escreverPatable(0x00, 0xC0);         // desligado = 0, ligado = potencia maxima
}

// No modo assincrono, o CC1101 transmite o que o ESP32 coloca no GDO0
void pulso(int nivel, int duracaoUs) {
  digitalWrite(PIN_GDO0, nivel);
  delayMicroseconds(duracaoUs);
}

// O controle de verdade comeca cada clique com alguns pulsos curtos
// e uma pausa, antes do primeiro quadro (vimos nas capturas):
//   +256 -757 +256 -757 ... -2700
// Inicio de cada clique: 4 pulsos curtos e um silencio de ~9,3 ms
void transmitirPreambulo() {
  for (int i = 0; i < PULSOS_PREAMBULO; i++) {
    pulso(HIGH, T_CURTO);
    pulso(LOW,  (i == PULSOS_PREAMBULO - 1) ? T_PAUSA_PREAMBULO : T_LONGO);
  }
}

// Um quadro = 1 bit de sincronismo (0) + 32 bits de dados.
//   0 = ligado curto + desligado longo
//   1 = ligado longo + desligado curto
// O ultimo bit termina num silencio de ~8,5 ms.
// (NAO existe cabecalho longo: aquilo era artefato do receptor)
void transmitirQuadro(uint32_t q) {
  pulso(HIGH, T_CURTO);                // bit de sincronismo
  pulso(LOW,  T_LONGO);
  for (int i = 31; i >= 0; i--) {
    bool bit = (q >> i) & 1;
    pulso(HIGH, bit ? T_LONGO : T_CURTO);
    pulso(LOW,  (i == 0) ? T_PAUSA_QUADRO : (bit ? T_CURTO : T_LONGO));
  }
}

void enviarComando(uint8_t cmd, const char* descricao) {
  uint32_t q = montarQuadro(cmd, contador);

  Serial.print("Enviando ");
  Serial.print(descricao);
  Serial.print(" (cmd ");
  Serial.print(cmd);
  Serial.print(", contador ");
  Serial.print(contador);
  Serial.print("): ");
  imprimirHex(q);
  Serial.println();

  pinMode(PIN_GDO0, OUTPUT);
  digitalWrite(PIN_GDO0, LOW);
  radio.sendCmd(0x35);                 // STX: liga o transmissor

  // Espera o chip terminar de calibrar e entrar em TX (estado 19).
  // Estados 8 e 9 = ainda calibrando/estabilizando.
  uint8_t estado = 0;
  unsigned long limite = millis() + 50;
  do {
    estado = lerStatus(0x35) & 0x1F;   // MARCSTATE
  } while (estado != 19 && millis() < limite);
  Serial.print("  estado do radio: ");
  Serial.print(estado);
  Serial.println(estado == 19 ? " (transmitindo)" : " (ATENCAO: nao esta em TX)");

  if (estado != 19) {                  // nao conseguiu entrar em TX: aborta
    radio.idle();
    pinMode(PIN_GDO0, INPUT);
    return;                            // nao avanca o contador
  }

  transmitirPreambulo();
  for (int r = 0; r < REPETICOES; r++) transmitirQuadro(q);

  digitalWrite(PIN_GDO0, LOW);
  radio.idle();                        // desliga o transmissor
  pinMode(PIN_GDO0, INPUT);

  contador = (contador + 1) & 0x07;    // proximo clique, proximo contador
}


// ============================================================
//  ENTRADA: SERIAL E BOTAO
// ============================================================
void mostrarAjuda() {
  Serial.println("Comandos disponiveis:");
  Serial.println("  varrer\tTesta Luz com todos os contadores (0 a 7)");
  Serial.println("  cont N\tAjusta o contador (ex: cont 5)");
  Serial.println("  teste\tTransmite Luz sem parar por 10 s");
  for (int i = 0; i < TOTAL_COMANDOS; i++) {
    Serial.print("  ");
    Serial.print(comandos[i].nome);
    Serial.print("\t");
    Serial.println(comandos[i].descricao);
  }
}

void processarSerial() {
  if (!Serial.available()) return;
  String texto = Serial.readStringUntil('\n');
  texto.trim();
  texto.toLowerCase();
  if (texto.length() == 0) return;

  if (texto == "ajuda") { mostrarAjuda(); return; }

  // "cont N": ajusta o contador manualmente (0 a 7)
  if (texto.startsWith("cont ")) {
    contador = texto.substring(5).toInt() & 0x07;
    Serial.print("Contador ajustado para ");
    Serial.println(contador);
    return;
  }

  // "teste": transmite Luz sem parar por 10 s (para outro receptor,
  // como o Broadlink ou um SDR, confirmar que o sinal esta saindo)
  if (texto == "teste") {
    Serial.println("Transmitindo Luz continuamente por 10 s...");
    uint32_t q = montarQuadro(8, contador);
    pinMode(PIN_GDO0, OUTPUT);
    digitalWrite(PIN_GDO0, LOW);
    radio.sendCmd(0x35);
    delay(5);
    unsigned long fim = millis() + 10000;
    while (millis() < fim) transmitirQuadro(q);
    digitalWrite(PIN_GDO0, LOW);
    radio.idle();
    pinMode(PIN_GDO0, INPUT);
    Serial.println("Fim do teste.");
    return;
  }

  // "varrer": manda Luz com cada contador (0 a 7), com pausa,
  // para descobrir qual o ventilador aceita
  if (texto == "varrer") {
    Serial.println("Varrendo contadores. Observe quando a luz mudar!");
    for (int c = 0; c < 8; c++) {
      contador = c;
      enviarComando(8, "Luz ON/OFF");
      delay(3000);
    }
    Serial.println("Fim da varredura.");
    return;
  }

  for (int i = 0; i < TOTAL_COMANDOS; i++) {
    if (texto == comandos[i].nome) {
      enviarComando(comandos[i].codigo, comandos[i].descricao);
      return;
    }
  }
  Serial.print("Comando desconhecido: ");
  Serial.println(texto);
  Serial.println("Digite \"ajuda\" para ver a lista.");
}

bool botaoClicado() {
  static int estadoAnterior = HIGH;
  static unsigned long ultimaMudanca = 0;
  int estado = digitalRead(PIN_BOTAO);
  if (estado != estadoAnterior && millis() - ultimaMudanca > 50) {
    ultimaMudanca = millis();
    estadoAnterior = estado;
    return estado == LOW;
  }
  return false;
}


// ============================================================
//  SETUP E LOOP
// ============================================================
void setup() {
  Serial.begin(115200);
  delay(1000);

  pinMode(PIN_BOTAO, INPUT_PULLUP);
  pinMode(PIN_CSN, OUTPUT);
  digitalWrite(PIN_CSN, HIGH);
  SPI.begin(PIN_SCK, PIN_MISO, PIN_MOSI, PIN_CSN);

  if (radio.begin() == STATUS_CHIP_NOT_FOUND) {
    Serial.println("CC1101 nao encontrado. Confira os fios.");
    while (true) delay(1000);
  }
  Serial.println("CC1101 encontrado!");

  autoTeste();
  configurarTransmissor();

  Serial.println();
  Serial.println("Pronto! Botao D22 = Luz ON/OFF, ou digite um comando.");
  mostrarAjuda();
}

void loop() {
  processarSerial();
  if (botaoClicado()) enviarComando(8, "Luz ON/OFF");
}
