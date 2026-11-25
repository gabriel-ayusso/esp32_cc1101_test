# Log

## 2026-11-25

* **README**: visao geral do projeto, montagem, compilacao, uso e integracao com o HA.

## 2026-11-24

* **Broadlink resolvido**: codigos gerados em 433,92 MHz com 2 variantes alternadas; pacote do HA em `ha/packages/`.
* **Frequencia**: o "433,00 MHz" era estimativa do aprendizado do Broadlink; o controle real esta em ~433,92.
* **Projeto criado**: sketches movidos para `firmware/` e documentacao em OKF.
* **Transmissao funcionando**: ESP32 + CC1101 controla o ventilador do escritorio em 100% dos envios, com qualquer contador.
* **Formato corrigido com o Broadlink**: o "cabecalho de 5,7 ms" visto no CC1101 era artefato do AGC; o quadro real tem bit de sincronismo e pausa de 8,5 ms.
* **Protocolo decifrado**: ID 0x4119, comando de 5 bits, contador de 3 bits, verificacao de 4 bits; formula validada em 30 capturas.
* **Pino errado**: o GDO0 estava no D4 mas o codigo lia o GPIO 16; toda analise anterior foi feita com sinal deformado.

## 2026-11-23

* **Montagem inicial**: ESP32 DevKit V1 reconhecido (cabo do Kindle era so de carga), OLED e botao funcionando.
