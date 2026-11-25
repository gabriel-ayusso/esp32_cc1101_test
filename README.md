# Remote Control RF 433 MHz

Controle dos ventiladores de teto da casa pelos codigos RF 433 MHz dos
controles remotos originais, com **ESP32 + CC1101** e **Broadlink RM4 Pro**,
integrado ao **Home Assistant** (e a Alexa).

O protocolo do controle foi decifrado por engenharia reversa: um quadro OOK de
32 bits com ID do controle, comando de 5 bits, contador de 3 bits e
verificacao de 4 bits. Com isso, os codigos sao **gerados** em vez de
apenas gravados, e funcionam em 100% dos envios.

## Situacao atual

| Item | Estado |
|---|---|
| Protocolo do ventilador | Decifrado e validado em 30 capturas |
| ESP32 + CC1101 transmitindo | Funcionando (qualquer contador) |
| Broadlink RM4 Pro pelo Home Assistant | Funcionando com codigos gerados em 433,92 MHz |
| Ventilador do escritorio | Mapeado (ID `0x4119`, 21 comandos) |
| Alexa | Via Emulated Hue (local, sem nuvem) |

## Estrutura

```
firmware/
  rf_rx/        receptor e analisador (captura e mapeamento guiado de controles)
  rf_tx/        transmissor que imita o controle do ventilador
  */archive/    versoes antigas, so para referencia
tools/
  gerar_pacote_broadlink.py   gera o pacote do HA com os codigos Broadlink
ha/packages/
  ventilador_escritorio.yaml  fan, luz e script de comandos para o HA
  alexa.yaml                  entidades expostas para a Alexa
docs/           documentacao completa (comece por docs/index.md)
```

## Hardware

* ESP32 DevKit V1 (clone DOIT, USB CH9102X)
* Modulo CC1101 433 MHz (alimentacao **somente 3,3 V**)
* Botao tactil no D22

| CC1101 | GND | VCC | GDO0 | CSN | SCK | MOSI | MISO | GDO2 |
|---|---|---|---|---|---|---|---|---|
| ESP32 | GND | 3V3 | D4 | D5 | D18 | D23 | D19 | D2 (nao usado) |

Detalhes e cuidados em [docs/hardware/montagem.md](docs/hardware/montagem.md).

## Compilar e gravar

Cada pasta de firmware abre tanto na **Arduino IDE** quanto no **PlatformIO**.

**PlatformIO** (terminal, dentro de `firmware/rf_rx` ou `firmware/rf_tx`):

```sh
pio run              # compila
pio run -t upload    # grava
pio device monitor   # Serial Monitor (115200)
```

No VS Code, abra `remote_control.code-workspace` e escolha o projeto na barra do PlatformIO.

**Arduino IDE**: placa `DOIT ESP32 DEVKIT V1`, Upload Speed **115200**
(921600 falha com o CH9102X). Instale a biblioteca
[CC1101 (mfurga)](https://github.com/mfurga/cc1101).

> A SmartRC-CC1101-Driver-Lib **nao** funciona nesta placa.

## Uso rapido

**rf_tx**: no Serial Monitor (115200, New Line), digite um comando:
`luz`, `off`, `fanoff`, `v1` a `v6`, `vento`, `rf`, `mudo`, `brilho+`,
`brilho-`, `k+`, `k-`, `k`, `lua`, `1h`, `2h`, `4h` (ou `ajuda`).
O botao D22 liga e desliga a luz.

**rf_rx**: aperte o controle perto da antena para ver os quadros decodificados.
Para mapear um controle novo, ligue `MODO_MAPEAMENTO` no topo do sketch.

## Home Assistant

1. Gere o pacote (ou use o que ja esta em `ha/packages/`):

   ```sh
   python3 tools/gerar_pacote_broadlink.py --id 0x4119 --nome ventilador_escritorio \
       --remote remote.universal_remote > ha/packages/ventilador_escritorio.yaml
   ```

2. Copie os arquivos de `ha/packages/` para `/config/packages/` e garanta no `configuration.yaml`:

   ```yaml
   homeassistant:
     packages: !include_dir_named packages
   ```

3. Reinicie o HA. Para mandar um comando:

   ```yaml
   action: script.ventilador_escritorio_comando
   data:
     comando: luz
   ```

Cada comando tem 2 variantes (contador 0 e 1) que o script alterna, porque o
ventilador ignora um quadro com o mesmo contador do ultimo recebido.

## Documentacao

| Assunto | Pagina |
|---|---|
| Indice geral | [docs/index.md](docs/index.md) |
| Historico de descobertas | [docs/log.md](docs/log.md) |
| Protocolo do ventilador | [docs/protocolos/ventilador-teto-433.md](docs/protocolos/ventilador-teto-433.md) |
| Codigos do ventilador do escritorio | [docs/dispositivos/ventilador-escritorio.md](docs/dispositivos/ventilador-escritorio.md) |
| Firmware de recepcao | [docs/firmware/rf-rx.md](docs/firmware/rf-rx.md) |
| Firmware de transmissao | [docs/firmware/rf-tx.md](docs/firmware/rf-tx.md) |
| Broadlink RM4 Pro | [docs/integracoes/broadlink-rm4.md](docs/integracoes/broadlink-rm4.md) |
| Problemas ja resolvidos | [docs/guias/troubleshooting.md](docs/guias/troubleshooting.md) |
