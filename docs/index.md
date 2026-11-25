---
okf_version: "0.2"
---

# Remote Control (RF 433 MHz)

Controle dos ventiladores de teto da casa por ESP32 + CC1101.

## Hardware

* [Montagem](/hardware/montagem.md): ligacao do ESP32, CC1101 e botao.

## Protocolos

* [Ventilador de teto 433 MHz](/protocolos/ventilador-teto-433.md): formato do quadro de 32 bits, contador e verificacao.

## Dispositivos

* [Ventilador do escritorio](/dispositivos/ventilador-escritorio.md): ID 0x4119, tabela completa de codigos.

## Firmware

* [rf_rx](/firmware/rf-rx.md): receptor, analisador e mapeamento guiado.
* [rf_tx](/firmware/rf-tx.md): transmissor que imita o controle.

## Integracoes

* [Broadlink RM4 Pro](/integracoes/broadlink-rm4.md): formato dos codigos aprendidos e limitacoes.

## Guias

* [Troubleshooting](/guias/troubleshooting.md): problemas ja resolvidos.
