---
type: Firmware
title: rf_rx (receptor e analisador)
description: Sketch que coloca o CC1101 em recepcao OOK crua e decodifica, mostra pulsos ou mapeia controles de forma guiada.
resource: /firmware/rf_rx/rf_rx.ino
tags: [firmware, esp32, cc1101, recepcao]
status: stable
generated: {by: claude-opus-5.5, at: "2026-11-24T21:15:00-03:00"}
---

# Como funciona

* O CC1101 fica em modo serial assincrono: entrega o sinal demodulado cru no GDO0.
* O ESP32 monitora o RSSI; quando passa de `LIMITE_DBM` (-55 dBm), grava os pulsos do GDO0 com filtro de ruido.
* O ganho (AGC) e congelado durante a captura.

# Chaves no topo do arquivo

| Chave | Valores |
|---|---|
| `MODO_MAPEAMENTO` | `true`: roteiro guiado pelo Serial Monitor (pede cada botao pelo nome e imprime um resumo) |
| `OOK_SAIDA` | `0` quadros em hex, `1` bits, `2` pulsos crus em us |
| `experimentos[]` | lista de modulacoes; o botao D22 alterna entre elas (OOK cru, FSK cru, modos pacote) |

# Uso

1. Gravar com Upload Speed 115200; Serial Monitor em 115200.
2. Apertar o controle perto da antena.
3. Para mapear um controle novo, ligar `MODO_MAPEAMENTO` e editar o array `roteiro[]`.

# Limitacoes

* O decodificador hex espera o falso cabecalho de 5,7 ms (artefato do AGC). Para sinais muito diferentes, usar `OOK_SAIDA = 2`.
* Medicoes de tempo do CC1101 sao menos fieis que as do Broadlink, especialmente no primeiro e ultimo pulso de cada quadro.
