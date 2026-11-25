# AGENTS.md

Projeto pessoal do Gabriel: controlar controles remotos RF 433 MHz da casa
(ventiladores de teto) com ESP32 + CC1101, com o objetivo final de integrar
no Home Assistant.

## Onde encontrar as coisas

| Preciso de... | Veja |
|---|---|
| Visao geral e indice da documentacao | `docs/index.md` |
| Historico do que foi feito e descoberto | `docs/log.md` |
| Ligacao dos fios (ESP32, CC1101, botao) | `docs/hardware/montagem.md` |
| Protocolo do controle de ventilador (formato do quadro, formula da verificacao) | `docs/protocolos/ventilador-teto-433.md` |
| Codigos do ventilador do escritorio (todos os comandos e contadores) | `docs/dispositivos/ventilador-escritorio.md` |
| Como usar o firmware de recepcao (mapear controles) | `docs/firmware/rf-rx.md` |
| Como usar o firmware de transmissao (imitar o controle) | `docs/firmware/rf-tx.md` |
| Broadlink RM4 Pro: formato dos codigos e por que falha | `docs/integracoes/broadlink-rm4.md` |
| Problemas ja resolvidos e armadilhas | `docs/guias/troubleshooting.md` |
| Pacote do Home Assistant (Broadlink) | `ha/packages/` (gerado por `tools/gerar_pacote_broadlink.py`) |

## Codigo

- `firmware/rf_rx/rf_rx.ino`: receptor/analisador (Arduino IDE, placa DOIT ESP32 DEVKIT V1).
- `firmware/rf_tx/rf_tx.ino`: transmissor que imita o controle do ventilador.
- `firmware/*/archive/`: versoes antigas, so para referencia (nao compilam junto).
- Cada pasta de firmware e tambem um projeto **PlatformIO** (`platformio.ini` com `src_dir = .`),
  entao o mesmo `.ino` abre na Arduino IDE e no VS Code.
- Compilar e gravar pelo terminal, dentro da pasta do firmware:
  `pio run` (compila), `pio run -t upload` (grava), `pio device monitor` (serial).
- No VS Code, abrir `remote_control.code-workspace` e escolher o projeto rf_rx ou rf_tx na barra do PlatformIO.

## Regras para agentes

- A documentacao em `docs/` segue o Open Knowledge Format (OKF v0.2):
  todo `.md` (exceto `index.md` e `log.md`) tem frontmatter YAML com `type`.
  Mantenha esse formato ao criar ou editar paginas, e atualize `docs/index.md`.
- Registre descobertas e mudancas relevantes em `docs/log.md` (mais recente primeiro).
- Escreva em portugues do Brasil e nao use travessoes (em dash ou en dash) nos textos.
- Upload na Arduino IDE: Upload Speed 115200 (921600 falha com o CH9102X).
- Biblioteca de radio usada: `CC1101` (mfurga). A SmartRC-CC1101-Driver-Lib NAO funciona nesta placa.
- Ao mapear um controle novo, prefira capturar com o Broadlink (mais fiel) e
  confirmar com o `rf_rx`. Documente em `docs/dispositivos/`.
