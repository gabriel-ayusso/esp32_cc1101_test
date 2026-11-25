---
type: Protocol
title: Protocolo do controle de ventilador de teto 433 MHz
description: Formato do quadro OOK de 32 bits com ID, comando, contador e verificacao, decifrado por engenharia reversa.
tags: [rf, 433mhz, ook, protocolo, ventilador]
status: stable
generated: {by: claude-opus-5.5, at: "2026-11-24T21:15:00-03:00"}
verified: [{by: "human:gabriel", at: "2026-11-24T20:43:00-03:00"}]
---

# Camada de radio

* Modulacao **OOK** (liga/desliga a portadora).
* Frequencia: **433,92 MHz**. O aprendizado do Broadlink estima 433,00 MHz, mas retransmitir nessa frequencia falha com frequencia.
* Unidade de tempo: pulso curto ~260 us, longo ~790 us.

# Estrutura de um clique

```
preambulo:  4x (curto ligado, longo desligado), ultimo desligado ~9,3 ms
quadro:     sync + 32 bits, termina com silencio de ~8,5 ms
            (o quadro se repete enquanto o botao esta apertado; 6 a 8 vezes num toque)
```

* **sync**: um bit 0 (ligado curto + desligado longo).
* **bit 0**: ligado 260 us + desligado 790 us.
* **bit 1**: ligado 790 us + desligado 260 us.
* NAO existe cabecalho longo. O receptor CC1101, em modo cru, mostra um falso "cabecalho" de ~5,7 ms porque o AGC le ruido como portadora durante o silencio.

# Quadro de 32 bits

```
byte 1-2 : ID do controle (ex: 0x41 0x19)
byte 3   : 111 + comando (5 bits)          = 0xE0 | cmd
byte 4   : contador (3 bits) + verif (4 bits) + 0
```

# Contador

* 3 bits, 0 a 7, **global**: avanca 1 a cada clique de qualquer botao.
* O ventilador so ignora um quadro com o **mesmo contador do ultimo recebido**. Qualquer outro valor e aceito.
* Por isso replay de codigo fixo (Broadlink) funciona uma vez e falha na repeticao.

# Verificacao

```
verif = (11 - ((cmd >> 1) & 7) - contador + 8 * (bit0(cmd) XOR bit4(cmd))) mod 16
byte4 = (contador << 5) | (verif << 1)
```

Validada em 30 de 30 capturas do controle do escritorio. A constante 11 pode depender do ID; confirmar ao mapear outro controle.

# Examples

Luz ON/OFF (cmd 8), contador 3: `41 19 E8 68`.
Velocidade 6 (cmd 12), contador 6: `(12>>1)&7 = 6`, termo do 8 = 0, `11-6-6 = -1 -> 15`, byte4 = `0xC0 | 0x1E` = `DE`, quadro `41 19 EC DE`.
