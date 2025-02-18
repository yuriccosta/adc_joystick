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
#define JOY_X 27 // PDF ESTÁ TROCADO
#define JOY_Y 26
#define SW_PIN 22
#define BUTTON_PIN_A 5          // Pino GPIO conectado ao botão A
#define zona_morta 100

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



void pwm_setup(uint pino) {
    gpio_set_function(pino, GPIO_FUNC_PWM);         // Configura o pino como saída PWM
    uint slice = pwm_gpio_to_slice_num(pino);         // Obtém o slice correspondente

    // Configura o divisor de clock:
    //pwm_set_clkdiv(slice, 4.0);
    
    pwm_set_wrap(slice, 4095);

    pwm_set_enabled(slice, true);  // Habilita o slice PWM
}

void configuraGPIO(){
    // Configuração do LED RGB
    pwm_setup(LED_PIN_RED);

    pwm_setup(LED_PIN_BLUE);

    gpio_init(LED_PIN_GREEN);
    gpio_set_dir(LED_PIN_GREEN, GPIO_OUT);

    // Configura os botões
    gpio_init(BUTTON_PIN_A);
    gpio_set_dir(BUTTON_PIN_A, GPIO_IN);
    gpio_pull_up(BUTTON_PIN_A);

    gpio_init (SW_PIN);
    gpio_set_dir(SW_PIN, GPIO_IN);
    gpio_pull_up(SW_PIN);
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



    // Configura os LEDs e botões
    configuraGPIO();

    // Configuração do ADC
    adc_init();
    adc_gpio_init(JOY_X);
    adc_gpio_init(JOY_Y);


    // Configuração da interrupção
    gpio_set_irq_enabled_with_callback(BUTTON_PIN_A, GPIO_IRQ_EDGE_FALL, true, &gpio_irq_handler);

    uint32_t last_print_time = 0; 

    stdio_init_all();
    while (true){
        adc_select_input(1);  
        uint16_t vrx_value = adc_read(); 
        adc_select_input(0);  
        uint16_t vry_value = adc_read(); 


        // Aumenta os valores do pwm ao se aproximar dos extremos do eixo x e y
        uint16_t pwm_x = (abs(vrx_value - 2048) > zona_morta) ? abs(vrx_value - 2048) : 0;
        uint16_t pwm_y = (abs(vry_value - 2048) > zona_morta) ? abs(vry_value - 2048) : 0;

        pwm_set_gpio_level(LED_PIN_RED, pwm_x); 
        pwm_set_gpio_level(LED_PIN_BLUE, pwm_y);

        float duty_cycle_x = (vrx_value / 4095.0) * 100;  
        float duty_cycle_y = (vry_value / 4095.0) * 100;

        
        uint32_t current_time = to_ms_since_boot(get_absolute_time());  
        if (current_time - last_print_time >= 1000) {  
            printf("VRX: %u\n", vrx_value); 
            printf("VRY: %u\n", vry_value);
            printf("Duty Cycle LED: %.2f%%\n", duty_cycle_x); 
            printf("Duty Cycle LED: %.2f%%\n", duty_cycle_y);
            last_print_time = current_time;  
        }

        sleep_ms(100); 
    }

    return 0;
}