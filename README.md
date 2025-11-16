# ufabc-sistemas-microprocessados
Repositório para controle de versão de códigos realizados em laboratório.

# Documentação
- Criar diagrama de blocos das funções

# Projeto
## Objetivo
Fazer protocolo de comunicação UART entre duas placas.

## Botões
1. __Cronômetro__: Ao clicar deve exibir cronômetro interno no display;
1. __ADC com sensor de luz__: Ao clicar deve exibir o sensor analógico no display;
1. __UART__: Ao clicar deve iniciar pedido de comunicação UART com outra placa.

## Protocolo de comunicação
- O cliente deve enviar um 'request' para a placa destino;
- Placa destino deve responder 'ok' para o cliente;
- Cliente deve enviar cronômetro interno em tempo real para placa destino;
- Placa destino continua contagem do cronômetro interno (da placa destino) em background;
- Placa destino mostra contagem da placa cliente no display;
- Ao soltar o botão da placa cliente a comunicação é encerrada e a placa volta a modo listener.


