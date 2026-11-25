---
type: Hardware Setup
title: Montagem ESP32 + CC1101
description: Ligacao dos fios entre ESP32 DevKit V1, modulo CC1101 433 MHz e botao tactil.
tags: [esp32, cc1101, fiacao, 433mhz]
status: stable
generated: {by: claude-opus-5.5, at: "2026-11-24T21:15:00-03:00"}
---

# Componentes

* **ESP32 DevKit V1** (clone DOIT), chip USB **CH9102X**, conector micro USB.
  Na Arduino IDE: placa `DOIT ESP32 DEVKIT V1`, Upload Speed **115200**.
* **CC1101 433 MHz V2.0** (modulo azul, cristal 26 MHz, antena SMA). Alimentacao **somente 3,3 V**.
* **Botao tactil** no protoboard (atravessando a canaleta central).

# Ligacao

Pino 1 do CC1101 e o de solda quadrada. Os pinos sao 2x4: ligar com jumpers femea-macho, nao encaixar no protoboard.

| CC1101 | Funcao | ESP32 |
|---|---|---|
| 1 | GND | GND |
| 2 | VCC | 3V3 |
| 3 | GDO0 | D4 (GPIO 4) |
| 4 | CSN | D5 |
| 5 | SCK | D18 |
| 6 | MOSI | D23 |
| 7 | MISO | D19 |
| 8 | GDO2 | D2 (nao usado) |

| Botao | ESP32 |
|---|---|
| uma perna | D22 |
| perna na diagonal | GND |

# Observacoes

* O D2 influencia o boot do ESP32; se o upload falhar, desconecte o GDO2.
* Use cabo USB que passe dados (o do Kindle era so de carga).
* Jumpers longos (mais de 15 cm) podem causar erro no SPI.
