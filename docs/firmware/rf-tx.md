---
type: Firmware
title: rf_tx (transmissor)
description: Sketch que monta o quadro do protocolo do ventilador e transmite pelo CC1101, comandado por botao ou Serial.
resource: /firmware/rf_tx/rf_tx.ino
tags: [firmware, esp32, cc1101, transmissao]
status: stable
generated: {by: claude-opus-5.5, at: "2026-11-24T21:15:00-03:00"}
verified: [{by: "human:gabriel", at: "2026-11-24T20:43:00-03:00"}]
---

# Como funciona

* CC1101 em OOK serial assincrono; o ESP32 gera o sinal no GDO0 com `delayMicroseconds`.
* `PATABLE[0] = 0x00` (desligado), `PATABLE[1] = 0xC0` (ligado, potencia maxima), `FREND0 = 0x11`.
* Antes de transmitir, espera o MARCSTATE chegar a 19 (TX); estados 8 e 9 sao calibracao.
* Envia preambulo + 8 repeticoes do quadro. O contador avanca a cada envio.
* Na inicializacao roda um auto-teste da formula contra quadros capturados.

# Comandos (Serial Monitor, 115200, New Line)

`luz`, `off`, `fanoff`, `v1` a `v6`, `vento`, `rf`, `mudo`, `brilho+`, `brilho-`, `k+`, `k-`, `k`, `lua`, `1h`, `2h`, `4h`.

Diagnostico:

* `varrer`: manda Luz com os contadores 0 a 7, com 3 s entre eles.
* `cont N`: ajusta o contador.
* `teste`: transmite Luz sem parar por 10 s (para outro receptor detectar).
* `ajuda`: lista os comandos.

Botao D22 manda Luz ON/OFF.

# Configuracao

* `ID_CONTROLE`: trocar para imitar outro controle do mesmo tipo.
* Frequencia fixa em 433,92 MHz (centro correto).
