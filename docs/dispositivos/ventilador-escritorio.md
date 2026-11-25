---
type: Device
title: Ventilador do escritorio
description: Ventilador de teto com luz LED do escritorio; ID 0x4119 e tabela completa de codigos validada com ESP32 + CC1101.
tags: [ventilador, escritorio, 433mhz, codigos]
status: stable
generated: {by: claude-opus-5.5, at: "2026-11-24T21:15:00-03:00"}
verified: [{by: "human:gabriel", at: "2026-11-24T20:43:00-03:00"}]
---

# Resumo

* Controle branco generico de ventilador de teto (botoes 1 a 6, FANOFF, R/F, timers 1H/2H/4H, cor K, luz noturna).
* Protocolo: [Ventilador de teto 433 MHz](/protocolos/ventilador-teto-433.md).
* **ID do controle: `0x4119`** (bytes 1-2 do quadro).
* Frequencia: ~433,92 MHz (o Broadlink estimou 433,00 ao aprender, mas isso faz o envio falhar).
* Status: transmissao pelo ESP32 + CC1101 funcionando em 100% dos envios ([rf_tx](/firmware/rf-tx.md)).
* Broadlink RM4 Pro: funcionando com codigos gerados e alternados ([detalhes](/integracoes/broadlink-rm4.md)).

# Codigos

Quadro de 32 bits em hex, para cada comando e cada valor do contador.
Ao transmitir, basta usar um contador diferente do ultimo enviado.
Todos os valores sao gerados pela formula, validada contra 30 capturas reais.

| Comando | Botão | cmd (dec) | cmd (bin) | cont 0 | cont 1 | cont 2 | cont 3 | cont 4 | cont 5 | cont 6 | cont 7 |
|---|---|---|---|---|---|---|---|---|---|---|---|
| `luz` | Luz ON/OFF | 8 | `01000` | `4119E80E` | `4119E82C` | `4119E84A` | `4119E868` | `4119E886` | `4119E8A4` | `4119E8C2` | `4119E8E0` |
| `off` | Desliga tudo | 6 | `00110` | `4119E610` | `4119E62E` | `4119E64C` | `4119E66A` | `4119E688` | `4119E6A6` | `4119E6C4` | `4119E6E2` |
| `fanoff` | Desliga ventilador | 22 | `10110` | `4119F600` | `4119F63E` | `4119F65C` | `4119F67A` | `4119F698` | `4119F6B6` | `4119F6D4` | `4119F6F2` |
| `v1` | Velocidade 1 | 16 | `10000` | `4119F006` | `4119F024` | `4119F042` | `4119F060` | `4119F09E` | `4119F0BC` | `4119F0DA` | `4119F0F8` |
| `v2` | Velocidade 2 | 18 | `10010` | `4119F204` | `4119F222` | `4119F240` | `4119F27E` | `4119F29C` | `4119F2BA` | `4119F2D8` | `4119F2F6` |
| `v3` | Velocidade 3 | 28 | `11100` | `4119FC1A` | `4119FC38` | `4119FC56` | `4119FC74` | `4119FC92` | `4119FCB0` | `4119FCCE` | `4119FCEC` |
| `v4` | Velocidade 4 | 10 | `01010` | `4119EA0C` | `4119EA2A` | `4119EA48` | `4119EA66` | `4119EA84` | `4119EAA2` | `4119EAC0` | `4119EAFE` |
| `v5` | Velocidade 5 | 15 | `01111` | `4119EF18` | `4119EF36` | `4119EF54` | `4119EF72` | `4119EF90` | `4119EFAE` | `4119EFCC` | `4119EFEA` |
| `v6` | Velocidade 6 | 12 | `01100` | `4119EC0A` | `4119EC28` | `4119EC46` | `4119EC64` | `4119EC82` | `4119ECA0` | `4119ECDE` | `4119ECFC` |
| `vento` | Vento natural | 21 | `10101` | `4119F512` | `4119F530` | `4119F54E` | `4119F56C` | `4119F58A` | `4119F5A8` | `4119F5C6` | `4119F5E4` |
| `rf` | Verao/inverno (R/F) | 7 | `00111` | `4119E700` | `4119E73E` | `4119E75C` | `4119E77A` | `4119E798` | `4119E7B6` | `4119E7D4` | `4119E7F2` |
| `mudo` | Bipe liga/desliga | 31 | `11111` | `4119FF08` | `4119FF26` | `4119FF44` | `4119FF62` | `4119FF80` | `4119FFBE` | `4119FFDC` | `4119FFFA` |
| `brilho+` | Brilho + | 1 | `00001` | `4119E106` | `4119E124` | `4119E142` | `4119E160` | `4119E19E` | `4119E1BC` | `4119E1DA` | `4119E1F8` |
| `brilho-` | Brilho - | 11 | `01011` | `4119EB1C` | `4119EB3A` | `4119EB58` | `4119EB76` | `4119EB94` | `4119EBB2` | `4119EBD0` | `4119EBEE` |
| `k+` | Cor mais fria | 13 | `01101` | `4119ED1A` | `4119ED38` | `4119ED56` | `4119ED74` | `4119ED92` | `4119EDB0` | `4119EDCE` | `4119EDEC` |
| `k-` | Cor mais quente | 14 | `01110` | `4119EE08` | `4119EE26` | `4119EE44` | `4119EE62` | `4119EE80` | `4119EEBE` | `4119EEDC` | `4119EEFA` |
| `k` | Troca cor 6000/3000/4000K | 30 | `11110` | `4119FE18` | `4119FE36` | `4119FE54` | `4119FE72` | `4119FE90` | `4119FEAE` | `4119FECC` | `4119FEEA` |
| `lua` | Luz noturna | 5 | `00101` | `4119E502` | `4119E520` | `4119E55E` | `4119E57C` | `4119E59A` | `4119E5B8` | `4119E5D6` | `4119E5F4` |
| `1h` | Timer 1h | 2 | `00010` | `4119E214` | `4119E232` | `4119E250` | `4119E26E` | `4119E28C` | `4119E2AA` | `4119E2C8` | `4119E2E6` |
| `2h` | Timer 2h | 9 | `01001` | `4119E91E` | `4119E93C` | `4119E95A` | `4119E978` | `4119E996` | `4119E9B4` | `4119E9D2` | `4119E9F0` |
| `4h` | Timer 4h | 25 | `11001` | `4119F90E` | `4119F92C` | `4119F94A` | `4119F968` | `4119F986` | `4119F9A4` | `4119F9C2` | `4119F9E0` |

# Observacoes

* A ordem dos codigos de velocidade nao segue a numeracao dos botoes (16, 18, 28, 10, 15, 12).
* O motor ligado gera ruido de RF que prejudica receptores fracos (afeta o Broadlink, nao o ESP32).
