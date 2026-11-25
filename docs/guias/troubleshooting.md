---
type: Guide
title: Troubleshooting
description: Problemas encontrados ao montar o projeto e como foram resolvidos.
tags: [troubleshooting, esp32, cc1101]
status: stable
generated: {by: claude-opus-5.5, at: "2026-11-24T21:15:00-03:00"}
---

# ESP32 nao aparece no Mac

* `ioreg -p IOUSB -l | grep -i "USB Product Name\|idVendor"` deve mostrar `USB Single Serial`, idVendor 6790.
* Se nao aparece: cabo so de carga. No macOS nao precisou de driver do CH9102X.

# Upload falha em "Unable to verify flash chip connection"

* Tools > Upload Speed: **115200**. Se travar em "Connecting...", segurar BOOT.

# SmartRC-CC1101-Driver-Lib diz "CC1101 nao encontrado"

* O chip responde por SPI puro (PARTNUM 0x00, VERSION 0x14). Usar a biblioteca `CC1101` (mfurga).
* `serialReceive()` da mfurga recusa o modo cru: configurar na mao (`IOCFG0 = 0x0D`, `PKTCTRL0 = 0x32`, strobe SRX `0x34`).
* `readReg` da mfurga nao le registradores de status (RSSI, MARCSTATE): ler com burst bit `0xC0` via SPI.

# Sinal recebido picotado / codigo "muda a cada clique"

* Causa real: o codigo lia o GPIO 16 mas o GDO0 estava no D4. Pino vizinho captava o sinal por acoplamento, deformado.
* Licao: confirmar o pino antes de concluir algo sobre o protocolo.

# Falso cabecalho de 5,7 ms

* Artefato do AGC do CC1101 no silencio entre quadros. Transmitir esse "cabecalho" faz o ventilador ignorar o comando.

# Transmissor nao funciona as vezes

* MARCSTATE 8/9 = ainda calibrando. Esperar o estado 19 antes de transmitir.
