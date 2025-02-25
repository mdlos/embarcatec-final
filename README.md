<img width=100% src="https://capsule-render.vercel.app/api?type=waving&color=02A6F4&height=120&section=header"/>
<h1 align="center">Embarcatec | Sistema Embarcado Simulador de Controle de Microondas com Display OLED e Matriz de LEDs</h1>

<div align="center">  
  <img width=40% src="http://img.shields.io/static/v1?label=STATUS&message=DESENVOLVIDO&color=02A6F4&style=for-the-badge"/>
</div>

## Descrição do Projeto

Este projeto consiste em um sistema embarcado que simula o funcionamento de um microondas, utilizando o microcontrolador RP2040 (Raspberry Pi Pico W). 
O sistema integra diversos periféricos, como um display OLED para exibição de informações, uma matriz de LEDs para indicadores visuais, um servo motor (simulador Wokwi) 
para simular o movimento do prato giratório, e um buzzer para fornecer feedback sonoro. A interação com o sistema é feita por meio de botões que permitem ao usuário configurar o 
tempo de cozimento e ajustar a potência do microondas.

## 🗒️ Lista de requisitos

- Microcontrolador: RP2040 (Raspberry Pi Pico W)
- Display OLED: I2C, endereço 0x3C
- Matriz de LEDs: WS2812
- Servo Motor: Simulado no Wokwi
- Buzzer: PWM para geração de tons
- Botões: GPIO para interação do usuário

## 🔧Pinagem

- GPIO 5: Botão A (incrementar tempo)
- GPIO 6: Botão B (ajustar potência)
- GPIO 22: Botão Joy (ligar/desligar microondas)
- GPIO 10: Servo motor (PWM)
- GPIO 11: LED verde (status do microondas)
- GPIO 12: LED azul (status adicional)
- GPIO 7: Matriz de LEDs WS2812
- GPIO 21: Buzzer (PWM)
- GPIO 14/15: I2C (SDA/SCL) para display OLED

## 💻 Como Utilizar
1. **Clone o repositório**;
2. **Abra o projeto na IDE de sua preferência (recomendado Visual Studio Code)**;
3. **Compile e carregue o firmware no microcontrolador RP2040**;
4. **Conecte os componentes conforme a pinagem descrita**;
5. **Interaja com o sistema utilizando os botões para configurar o tempo e a potência, e observe as informações no display OLED e matriz de LEDs**

## 🛠 Tecnologias

1. **Git e Github**;
2. **VScode**;
3. **Linguagem C**;
4. **Software de emulação PuTTy.**

## 🔧 Funcionalidades:

Display OLED: Exibe informações como tempo de cozimento e status do microondas.
Matriz de LEDs: Indica visualmente o estado do sistema e alerta para o fim do processo.
Servo Motor: Simula o movimento do prato giratório de acordo com o tempo de cozimento.
Buzzer: Emite alertas sonoros ao iniciar e finalizar o ciclo de cozimento.
Botões: Permitem ao usuário configurar o tempo de cozimento e ajustar a potência.

## 🗒️ Testes Realizados
Verificação do funcionamento dos botões.
Teste da exibição no display OLED e matriz de LEDs.
Validação do controle do servo motor e buzzer.
Teste do ciclo completo de cozimento.

## 🗒️ Conclusão

O projeto demonstrou a viabilidade de sistemas embarcados para aplicações do cotidiano, com a integração eficiente de hardware e software. 
O uso do RP2040 mostrou-se adequado para controlar múltiplos periféricos simultaneamente, proporcionando uma experiência interativa e educativa.
O sistema pode ser expandido e adaptado para outras aplicações em eletrônica e automação.

## 💻 Desenvolvedor
 
<table>
  <tr>
    <td align="center"><img style="" src="https://avatars.githubusercontent.com/u/72825281?v=4" width="100px;" alt=""/><br /><sub><b> Marcio Fonseca </b></sub></a><br />👨‍💻</a></td>
  </tr>
</table>


## 🎥 Demonstração: 
- Para ver o funcionamento do projeto, acesse o vídeo:

=======
# embarcatec3
