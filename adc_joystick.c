#include <stdlib.h>
#include "pico/stdlib.h"
#include "hardware/i2c.h"
#include "inc/ssd1306.h"
#include "inc/font.h"

#include <stdio.h>
#include <hardware/pio.h>           
#include "hardware/clocks.h"        
#include "hardware/gpio.h"
#include "hardware/pwm.h"
#include "hardware/adc.h"

#include "animacao_matriz.pio.h" // Biblioteca PIO para controle de LEDs WS2818B

// Definição de constantes
#define LED_PIN_GREEN 11
#define LED_PIN_BLUE 12
#define LED_PIN_RED 13
#define LED_COUNT 25            // Número de LEDs na matriz
#define MATRIZ_PIN 7            // Pino GPIO conectado aos LEDs WS2818B
#define BUTTON_PIN_A 5          // Pino GPIO conectado ao botão A
#define BUTTON_PIN_B 6          // Pino GPIO conectado ao botão B
#define I2C_PORT i2c1
#define I2C_SDA 14
#define I2C_SCL 15
#define endereco 0x3C


// Declaração de variáveis globais
PIO pio;
uint sm;
ssd1306_t ssd; // Inicializa a estrutura do display
static volatile uint32_t last_time = 0; // Variável para armazenar o tempo do último evento
static volatile uint green_state = 0; // Variável para armazenar o estado do LED verde
static volatile uint blue_state = 0; // Variável para armazenar o estado do LED azul


double padrao_led[10][LED_COUNT] = {
        {0, 1, 1, 1, 0,
         0, 1, 0, 1, 0,
         0, 1, 0, 1, 0,
         0, 1, 0, 1, 0,
         0, 1, 1, 1, 0,
        }, // 0
        {0, 0, 1, 0, 0,
         0, 1, 1, 0, 0,
         1, 0, 1, 0, 0,
         0, 0, 1, 0, 0,
         0, 0, 1, 0, 0,
        }, // 1
        {0, 1, 1, 1, 0,
         0, 0, 0, 1, 0,
         0, 0, 1, 0, 0,
         0, 1, 0, 0, 0,
         0, 1, 1, 1, 0,
        },  // 2
        {0, 1, 1, 1, 0,
         0, 0, 0, 1, 0,
         0, 1, 1, 1, 0,
         0, 0, 0, 1, 0,
         0, 1, 1, 1, 0,
        }, // 3
        {0, 1, 0, 1, 0,
         0, 1, 0, 1, 0,
         0, 1, 1, 1, 0,
         0, 0, 0, 1, 0,
         0, 0, 0, 1, 0,
        }, // 4
        {0, 1, 1, 1, 0,
         0, 1, 0, 0, 0,
         0, 1, 1, 1, 0,
         0, 0, 0, 1, 0,
         0, 1, 1, 1, 0,
        }, // 5
        {0, 1, 1, 1, 0,
         0, 1, 0, 0, 0,
         0, 1, 1, 1, 0,
         0, 1, 0, 1, 0,
         0, 1, 1, 1, 0,
        },  // 6
        {0, 1, 1, 1, 1,
         0, 0, 0, 0, 1,
         0, 0, 0, 1, 0,
         0, 0, 1, 0, 0,
         0, 0, 1, 0, 0,
        }, // 7
        {0, 1, 1, 1, 0,
         0, 1, 0, 1, 0,
         0, 1, 1, 1, 0,
         0, 1, 0, 1, 0,
         0, 1, 1, 1, 0,
        }, // 8
        {0, 1, 1, 1, 0,
         0, 1, 0, 1, 0,
         0, 1, 1, 1, 0,
         0, 0, 0, 1, 0,
         0, 1, 1, 1, 0,
        }  // 9
    };

// Ordem da matriz de LEDS, útil para poder visualizar na matriz do código e escrever na ordem correta do hardware
int ordem[LED_COUNT] = {0, 1, 2, 3, 4, 9, 8, 7, 6, 5, 10, 11, 12, 13, 14, 19, 18, 17, 16, 15, 20, 21, 22, 23, 24};  


//rotina para definição da intensidade de cores do led
uint32_t matrix_rgb(double r)
{
  // Para não ficar forte demais, a intensidade de cor é multiplicada por 50
  unsigned char R;
  R = r * 255; 
  return (R << 16);
}

void display_num(int number){
    uint32_t valor_led;

    for (int i = 0; i < LED_COUNT; i++){
        // Define a cor do LED de acordo com o padrão
        valor_led = matrix_rgb(padrao_led[number][ordem[24 - i]]);
        // Atualiza o LED
        pio_sm_put_blocking(pio, sm, valor_led);
    }
}


static void gpio_irq_handler(uint gpio, uint32_t events) {
     // Obtém o tempo atual em milissegundos
    uint32_t current_time = to_ms_since_boot(get_absolute_time());
    // Verificação de tempo para debounce
    if (current_time - last_time > 200){
        if(gpio == BUTTON_PIN_A){
            green_state = !green_state;
            gpio_put(LED_PIN_GREEN, green_state);

            char string[13];
            snprintf(string, 13, "Led verde %u\n", green_state);

            printf("Botão A pressionado\n");
            printf(string);

            // Atualiza o conteúdo do display com animações
            ssd1306_fill(&ssd, true); // Limpa o display
            ssd1306_rect(&ssd, 3, 3, 122, 58, false, true); // Desenha um retângulo
            ssd1306_draw_string(&ssd, "Botao A", 8, 10); // Desenha uma string
            ssd1306_draw_string(&ssd, "pressionado", 8, 20); // Desenha uma string
            ssd1306_draw_string(&ssd, string, 8, 30); // Desenha uma string
            ssd1306_send_data(&ssd); // Atualiza o display

        } else if(gpio == BUTTON_PIN_B){
            blue_state = !blue_state;
            gpio_put(LED_PIN_BLUE, blue_state);

            char string[13];
            snprintf(string, 13, "Led azul %u\n", blue_state);

            printf("Botão B pressionado\n");
            printf(string);

            // Atualiza o conteúdo do display com animações
            ssd1306_fill(&ssd, true); // Limpa o display
            ssd1306_rect(&ssd, 3, 3, 122, 58, false, true); // Desenha um retângulo
            ssd1306_draw_string(&ssd, "Botao B", 8, 10); // Desenha uma string
            ssd1306_draw_string(&ssd, "pressionado", 8, 20); // Desenha uma string
            ssd1306_draw_string(&ssd, string, 8, 30); // Desenha uma string
            ssd1306_send_data(&ssd); // Atualiza o display
        }

        last_time = current_time; // Atualiza o tempo do último evento
    }
}

int main(){
  // I2C Initialisation. Using it at 400Khz.
  i2c_init(I2C_PORT, 400 * 1000);

  gpio_set_function(I2C_SDA, GPIO_FUNC_I2C); // Set the GPIO pin function to I2C
  gpio_set_function(I2C_SCL, GPIO_FUNC_I2C); // Set the GPIO pin function to I2C
  gpio_pull_up(I2C_SDA); // Pull up the data line
  gpio_pull_up(I2C_SCL); // Pull up the clock line
  ssd1306_init(&ssd, WIDTH, HEIGHT, false, endereco, I2C_PORT); // Inicializa o display
  ssd1306_config(&ssd); // Configura o display
  ssd1306_send_data(&ssd); // Envia os dados para o display

  // Limpa o display. O display inicia com todos os pixels apagados.
  ssd1306_fill(&ssd, false);
  ssd1306_send_data(&ssd);

  
  // Configuração do PIO
  pio = pio0; 
  bool ok;
  
  //coloca a frequência de clock para 128 MHz, facilitando a divisão pelo clock
  ok = set_sys_clock_khz(128000, false);

  stdio_init_all(); // Inicializa a comunicação serial

  printf("iniciando a transmissão PIO");
  if (ok) printf("clock set to %ld\n", clock_get_hz(clk_sys));

  uint offset = pio_add_program(pio, &animacao_matriz_program);
  sm = pio_claim_unused_sm(pio, true);
  animacao_matriz_program_init(pio, sm, offset, MATRIZ_PIN);


  // Configuração do LED RGB
  gpio_init(LED_PIN_RED);
  gpio_set_dir(LED_PIN_RED, GPIO_OUT);

  gpio_init(LED_PIN_GREEN);
  gpio_set_dir(LED_PIN_GREEN, GPIO_OUT);

  gpio_init(LED_PIN_BLUE);
  gpio_set_dir(LED_PIN_BLUE, GPIO_OUT);


  // Configura os botões
  gpio_init(BUTTON_PIN_A);
  gpio_set_dir(BUTTON_PIN_A, GPIO_IN);
  gpio_pull_up(BUTTON_PIN_A);

  gpio_init(BUTTON_PIN_B);
  gpio_set_dir(BUTTON_PIN_B, GPIO_IN);
  gpio_pull_up(BUTTON_PIN_B);


  // Configuração da interrupção
  gpio_set_irq_enabled_with_callback(BUTTON_PIN_A, GPIO_IRQ_EDGE_FALL, true, &gpio_irq_handler);
  gpio_set_irq_enabled_with_callback(BUTTON_PIN_B, GPIO_IRQ_EDGE_FALL, true, &gpio_irq_handler);

    while (true){
        if (stdio_usb_connected()){ // Certifica-se de que o USB está conectado
            char c;
            if (scanf("%c", &c) == 1){ // Lê caractere da entrada padrão
                printf("Recebido: '%c'\n", c);
                
                if (c <= '9' && c >= '0'){
                    int num = c - '0';
                    display_num(num);
                }

                char string[2] = {c, '\0'};
                // Atualiza o conteúdo do display com animações
                ssd1306_fill(&ssd, true); // Limpa o display
                ssd1306_rect(&ssd, 3, 3, 122, 58, false, true); // Desenha um retângulo
                ssd1306_draw_string(&ssd, string, 8, 10); // Desenha uma string 
                ssd1306_send_data(&ssd); // Atualiza o display
            }
        }
        sleep_ms(100);
    }

    return 0;
}