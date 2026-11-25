#!/usr/bin/env python3
"""
Gera o pacote do Home Assistant com os codigos Broadlink (b64) de um
ventilador de teto do protocolo descrito em docs/protocolos/ventilador-teto-433.md.

Uso:
    python3 tools/gerar_pacote_broadlink.py                 # ventilador do escritorio
    python3 tools/gerar_pacote_broadlink.py --id 0x4119 --nome ventilador_escritorio \
        --remote remote.universal_remote > ha/packages/ventilador_escritorio.yaml

Cada comando ganha 2 variantes (contador 0 e 1). O script do HA alterna entre
elas com um input_boolean, porque o ventilador ignora um codigo igual ao ultimo.
"""
import argparse
import base64

COMANDOS = [
    ("luz", 8, "Luz ON/OFF"), ("off", 6, "Desliga tudo"), ("fanoff", 22, "Desliga ventilador"),
    ("v1", 16, "Velocidade 1"), ("v2", 18, "Velocidade 2"), ("v3", 28, "Velocidade 3"),
    ("v4", 10, "Velocidade 4"), ("v5", 15, "Velocidade 5"), ("v6", 12, "Velocidade 6"),
    ("vento", 21, "Vento natural"), ("rf", 7, "Verao/inverno"), ("mudo", 31, "Bipe"),
    ("brilho_mais", 1, "Brilho +"), ("brilho_menos", 11, "Brilho -"),
    ("k_mais", 13, "Cor mais fria"), ("k_menos", 14, "Cor mais quente"), ("k", 30, "Troca a cor"),
    ("lua", 5, "Luz noturna"), ("timer_1h", 2, "Timer 1h"), ("timer_2h", 9, "Timer 2h"),
    ("timer_4h", 25, "Timer 4h"),
]

# Tempos em unidades do Broadlink (269/8192 ms ~ 32,84 us)
CURTO, LONGO = 8, 24          # ~260 us e ~790 us
PAUSA_PREAMBULO = 283         # ~9,3 ms
PAUSA_QUADRO = 258            # ~8,5 ms
PAUSA_FINAL = 1500            # ~49 ms
FREQ_KHZ = 433920             # centro; 433000 (frequencia "aprendida") falha as vezes
REPETICOES = 6


def quadro(id_controle, cmd, cont):
    verif = (11 - ((cmd >> 1) & 7) - cont + 8 * ((cmd & 1) ^ ((cmd >> 4) & 1))) & 15
    return (id_controle << 16) | ((0xE0 | cmd) << 8) | (cont << 5) | (verif << 1)


def _dur(u):
    return bytes([u]) if u < 256 else bytes([0, u >> 8, u & 255])


def codigo_broadlink(id_controle, cmd, cont):
    q = quadro(id_controle, cmd, cont)
    d = FREQ_KHZ.to_bytes(4, "little")
    for i in range(4):                                  # preambulo
        d += _dur(CURTO) + _dur(LONGO if i < 3 else PAUSA_PREAMBULO)
    for r in range(REPETICOES):
        d += _dur(CURTO) + _dur(LONGO)                  # bit de sincronismo
        for i in range(31, -1, -1):
            bit = (q >> i) & 1
            if i == 0:
                pausa = PAUSA_FINAL if r == REPETICOES - 1 else PAUSA_QUADRO
            else:
                pausa = CURTO if bit else LONGO
            d += _dur(LONGO if bit else CURTO) + _dur(pausa)
    cab = bytes([0xB0, 0xC0, len(d) & 255, len(d) >> 8])
    return base64.b64encode(cab + d).decode()


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("--id", default="0x4119")
    ap.add_argument("--nome", default="ventilador_escritorio")
    ap.add_argument("--titulo", default="Ventilador do escritorio")
    ap.add_argument("--titulo-luz", dest="titulo_luz", default="Luz escritório")
    ap.add_argument("--titulo-fan", dest="titulo_fan", default="Ventilador escritório")
    ap.add_argument("--remote", default="remote.universal_remote")
    a = ap.parse_args()
    idc = int(a.id, 16)
    n = a.nome

    print(f"""# Pacote gerado por tools/gerar_pacote_broadlink.py (nao editar os codigos na mao)
# {a.titulo}: ID {a.id}, envio pelo Broadlink ({a.remote}) em 433,92 MHz.
# Instalar: copie para /config/packages/ e garanta no configuration.yaml:
#   homeassistant:
#     packages: !include_dir_named packages

input_boolean:
  {n}_alterna:
    name: "{a.titulo}: alterna codigo"
    icon: mdi:swap-horizontal
  {n}_luz_estado:
    name: "{a.titulo}: estado da luz"
    icon: mdi:lightbulb
  {n}_ligado:
    name: "{a.titulo}: ligado"
    icon: mdi:ceiling-fan

input_number:
  {n}_velocidade:
    name: "{a.titulo}: velocidade"
    icon: mdi:fan-speed-1
    min: 1
    max: 6
    step: 1
  {n}_percentual:
    name: "{a.titulo}: percentual pedido"
    icon: mdi:percent
    min: 1
    max: 100
    step: 1

# Luz "otimista": o ventilador nao informa o estado, entao o HA guarda o
# ultimo estado que ele mesmo mandou. O botao do controle fisico dessincroniza.
template:
  - light:
      - name: "{a.titulo_luz}"
        unique_id: {n}_luz
        state: "{{{{ is_state('input_boolean.{n}_luz_estado', 'on') }}}}"
        turn_on:
          - condition: state
            entity_id: input_boolean.{n}_luz_estado
            state: "off"
          - action: input_boolean.turn_on
            target:
              entity_id: input_boolean.{n}_luz_estado
          - action: script.{n}_comando
            data:
              comando: luz
        turn_off:
          - condition: state
            entity_id: input_boolean.{n}_luz_estado
            state: "on"
          - action: input_boolean.turn_off
            target:
              entity_id: input_boolean.{n}_luz_estado
          - action: script.{n}_comando
            data:
              comando: luz

  # Ventilador "otimista" (mesmo motivo da luz). 6 velocidades = degraus de ~17%.
  - fan:
      - name: "{a.titulo_fan}"
        unique_id: {n}_fan
        speed_count: 6
        state: "{{{{ is_state('input_boolean.{n}_ligado', 'on') }}}}"
        # Informa o percentual exato que foi pedido (a Alexa confere o valor
        # depois do comando e reclama se vier diferente, ex: pediu 30, voltou 33).
        # Tambem vale desligado: ao "ligar", a Alexa manda o ultimo valor e o
        # ventilador volta na mesma velocidade em vez de ir para a 6.
        percentage: "{{{{ states('input_number.{n}_percentual') | int(17) }}}}"
        # Quando o pedido ja traz um percentual (Alexa/Emulated Hue costuma mandar),
        # o HA chama turn_on E depois set_percentage. Para nao mandar 2 codigos
        # (2 bipes), o turn_on so envia quando NAO vier percentual.
        turn_on:
          - action: input_boolean.turn_on
            target:
              entity_id: input_boolean.{n}_ligado
          - condition: template
            value_template: "{{{{ percentage is not defined or percentage is none }}}}"
          - action: script.{n}_comando
            data:
              comando: "v{{{{ states('input_number.{n}_velocidade') | int(1) }}}}"
        turn_off:
          - action: input_boolean.turn_off
            target:
              entity_id: input_boolean.{n}_ligado
          - action: script.{n}_comando
            data:
              comando: fanoff
        set_percentage:
          - if:
              - condition: template
                value_template: "{{{{ percentage | int(0) == 0 }}}}"
            then:
              - action: input_boolean.turn_off
                target:
                  entity_id: input_boolean.{n}_ligado
              - action: script.{n}_comando
                data:
                  comando: fanoff
            else:
              - variables:
                  vel: "{{{{ [1, ((percentage | int) * 6 / 100) | round(0, 'ceil') | int] | max }}}}"
              - action: input_number.set_value
                target:
                  entity_id: input_number.{n}_velocidade
                data:
                  value: "{{{{ vel }}}}"
              - action: input_number.set_value
                target:
                  entity_id: input_number.{n}_percentual
                data:
                  value: "{{{{ percentage | int }}}}"
              - action: input_boolean.turn_on
                target:
                  entity_id: input_boolean.{n}_ligado
              - action: script.{n}_comando
                data:
                  comando: "v{{{{ vel }}}}"

script:
  {n}_comando:
    alias: "{a.titulo}: comando"
    description: "Envia um comando ao ventilador, alternando entre 2 variantes do codigo."
    mode: queued
    fields:
      comando:
        description: "Nome do comando"
        required: true
        example: luz
        selector:
          select:
            options:""")
    for nome, _, desc in COMANDOS:
        print(f'              - label: "{desc}"\n                value: "{nome}"')
    print(f"""    variables:
      variante: "{{{{ 1 if is_state('input_boolean.{n}_alterna', 'on') else 0 }}}}"
      codigos:""")
    for nome, cmd, _ in COMANDOS:
        print(f"        \"{nome}\":")
        for c in (0, 1):
            print(f"          - {codigo_broadlink(idc, cmd, c)}")
    print(f"""    sequence:
      - action: remote.send_command
        target:
          entity_id: {a.remote}
        data:
          command: "b64:{{{{ codigos[comando][variante | int] }}}}"
      - action: input_boolean.toggle
        target:
          entity_id: input_boolean.{n}_alterna""")


if __name__ == "__main__":
    main()
