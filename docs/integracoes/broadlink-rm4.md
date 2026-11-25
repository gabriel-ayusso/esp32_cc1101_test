---
type: Integration
title: Broadlink RM4 Pro
description: Formato interno dos codigos RF do Broadlink no Home Assistant e por que ele falha com o ventilador.
tags: [broadlink, home-assistant, rf]
status: stable
generated: {by: claude-opus-5.5, at: "2026-11-24T21:15:00-03:00"}
---

# Situacao

* Entidade no HA: `remote.universal_remote`.
* Funciona com codigos **gerados** (nao aprendidos), via pacote `ha/packages/ventilador_escritorio.yaml`.

# Por que o codigo aprendido falhava

* **Frequencia errada**: o aprendizado estimou 433,00 MHz e o RM4 retransmite nessa frequencia, fora do centro do receptor do ventilador. Com o motor ligado (ruido), falhava. Os codigos gerados usam 433,92 MHz.
* **Contador fixo**: o ventilador ignora um quadro com o mesmo contador do ultimo recebido. Mandar o mesmo codigo duas vezes seguidas: a primeira funciona, a segunda nao.

# Solucao adotada

* `tools/gerar_pacote_broadlink.py` gera 2 variantes por comando (contador 0 e 1).
* O script `script.ventilador_escritorio_comando` alterna entre elas com `input_boolean.ventilador_escritorio_alterna`.
* Uso: `action: script.ventilador_escritorio_comando` com `data: {comando: luz}`.

# Formato do codigo aprendido (RF)

Codigos ficam em `/config/.storage/broadlink_remote_<mac>_codes`, em base64.

```
byte 0     : tipo (0xb0/0xb1 vistos nos aprendidos)
byte 1     : 0xc0
bytes 2-3  : tamanho dos dados (little endian)
bytes 4-7  : frequencia em kHz (little endian), ex: 0x00069B68 = 433000
resto      : duracoes alternadas ligado/desligado, unidade 269/8192 ms (~32,84 us)
             valor > 255: 0x00 seguido de 2 bytes big endian
fim        : 0x00 0x05 0xDC (pausa final ~49 ms)
```

# Uso como segundo receptor

Aprender um sinal com `remote.learn_command` (`command_type: rf`) e decodificar o arquivo acima e a forma mais fiel de medir pulsos.
Foi assim que se descobriu o formato real do quadro (sync + 32 bits, sem cabecalho longo).
