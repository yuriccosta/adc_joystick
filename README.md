# Leitura do joystick e controle de leds e display

## Requisitos do Projeto

- **Microcontrolador**: Raspberry Pi Pico W
- **Display ssd1306**
- **Led RGB**
- **Joystick**
- **Ambiente de desenvolvimento**: VS Code + Pico SDK  
- **Simulação**: Wokwi integrada ao VS Code  

## Instruções de Uso

1. **Clone o repositório**:
    ```sh
    git clone https://github.com/yuriccosta/adc_joystick
    cd https://github.com/yuriccosta/adc_joystick
    ```

2. **Configure o ambiente de desenvolvimento**:
    - Instale a extensão do vscode: Raspberry Pi Pico e configure o ambiente conforme instruções.
    - Instale a extensão do vscode: Wokwi e configure o ambiente conforme instruções.

3. **Compile o código**:
    - Abra o projeto no VS Code.
    - Aperte no botão Compile da extensão do Pico SDK.

4. **Rode no wokwi integrado ao vscode**:
    - Abra o arquivo diagram.json.
    - Aperte no botão Run da extensão do Wokwi.


## Explicações
- Foi definido uma variavel para ser a zona morta, pois o joystick em hardware não fica exatamente no centro e varia bastante
- Foi alterado o wrap para 4066 pois o joystick em hardware não alcança 4095 e 0. Ele vai de 16 a 4081 no meu caso, e para fazer com que ele brilhe com sua intensidade máxima nos extremos, foi necessário que o wrap alterasse para (4081 - 16) e dessa forma o duty cycle consegue atingir 100% nas extremidades

## Vídeo de Demonstração
Assista à demonstração do projeto em execução:  
[Vídeo Demonstrativo](https://youtu.be/aGPeZtyERJ8)  
  
  
