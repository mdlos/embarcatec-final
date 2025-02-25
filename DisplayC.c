#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "pico/stdlib.h"
#include "hardware/i2c.h"
#include "hardware/pio.h"
#include "hardware/clocks.h"
#include "hardware/pwm.h"
#include "inc/ssd1306.h"
#include "inc/font.h"
#include "ws2812.pio.h"

// Definições dos pinos
#define BUTTON_A_PIN 5
#define BUTTON_B_PIN 6
#define BUTTON_JOY_PIN 22
#define SERVO_PIN 10
#define DUTY_CYCLE_90 90
#define DUTY_CYCLE_180 180

#define LED_G_GPIO 11
#define LED_B_GPIO 12
#define NUM_PIXELS 25
#define WS2812_PIN 7
#define BUZZER_PIN 21

#define I2C_PORT i2c1
#define I2C_SDA 14
#define I2C_SCL 15
#define endereco 0x3C

// Matriz de dígitos para exibição na matriz de LEDs
bool digitos[10][NUM_PIXELS] = {
   {0, 1, 1, 1, 0,  
    0, 1, 0, 1, 0,  
    0, 1, 0, 1, 0,  
    0, 1, 0, 1, 0,  
    0, 1, 1, 1, 0}, 

   {0, 0, 1, 0, 0,  
    0, 0, 1, 0, 0,  
    0, 0, 1, 0, 0,  
    0, 1, 1, 0, 0,  
    0, 0, 1, 0, 0}, 

   {0, 1, 1, 1, 0,  
    0, 1, 0, 0, 0,  
    0, 1, 1, 1, 0,  
    0, 0, 0, 1, 0,  
    0, 1, 1, 1, 0}, 

   {0, 1, 1, 1, 0,  
    0, 0, 0, 1, 0,  
    0, 1, 1, 1, 0,  
    0, 0, 0, 1, 0,  
    0, 1, 1, 1, 0}, 

   {0, 1, 0, 0, 0,  
    0, 0, 0, 1, 0,  
    0, 1, 1, 1, 0,  
    0, 1, 0, 1, 0,  
    0, 1, 0, 1, 0}, 

   {0, 1, 1, 1, 0,  
    0, 0, 0, 1, 0,  
    0, 1, 1, 1, 0,  
    0, 1, 0, 0, 0,  
    0, 1, 1, 1, 0}, 

   {0, 1, 1, 1, 0,  
    0, 1, 0, 1, 0,  
    0, 1, 1, 1, 0,  
    0, 1, 0, 0, 0,  
    0, 1, 1, 0, 0}, 

   {0, 1, 0, 0, 0,  
    0, 0, 0, 1, 0,  
    0, 1, 0, 0, 0,  
    0, 0, 0, 1, 0,  
    0, 1, 1, 1, 0}, 

   {0, 1, 1, 1, 0,  
    0, 1, 0, 1, 0,  
    0, 1, 1, 1, 0,  
    0, 1, 0, 1, 0,  
    0, 1, 1, 1, 0}, 

   {0, 1, 1, 1, 0,  
    0, 0, 0, 1, 0,  
    0, 1, 1, 1, 0,  
    0, 1, 0, 1, 0,  
    0, 1, 1, 1, 0}  
};

// Variáveis globais
bool led_green_state = false;
bool led_blue_state = false;

static uint32_t last_time_a = 0;
static uint32_t last_time_b = 0;
static uint32_t last_time_joy = 0;

static ssd1306_t ssd;
static PIO pio_global;
static uint sm_global;

static const uint32_t DEBOUNCE_DELAY = 600;

int tempo_cozimento = 0;
int potencia = 800;
bool microondas_ligado = false;

// Configuração do servo motor
void configurar_servo() {
    gpio_set_function(SERVO_PIN, GPIO_FUNC_PWM);
    uint slice_num = pwm_gpio_to_slice_num(SERVO_PIN);
    pwm_set_wrap(slice_num, 20000); // Período de 20ms (50Hz)
    pwm_set_enabled(slice_num, true);
}

// Função para mover o servo motor para uma posição específica
void mover_servo(int angulo) {
    uint slice_num = pwm_gpio_to_slice_num(SERVO_PIN);
    uint16_t pulso = (angulo * 1000 / 180) + 1000; // Converte ângulo para pulso em microssegundos
    pwm_set_gpio_level(SERVO_PIN, pulso);
}

// Função para controlar a velocidade do servo motor
void controlar_servo(int tempo_cozimento) {
    int velocidade = 1000 / (tempo_cozimento + 1); // Ajuste a velocidade com base no tempo
    mover_servo(potencia == 800 ? DUTY_CYCLE_90 : DUTY_CYCLE_180);
    sleep_ms(velocidade);
}

// Configuração do buzzer
void configurar_buzzer() {
    gpio_set_function(BUZZER_PIN, GPIO_FUNC_PWM);
    uint slice_num = pwm_gpio_to_slice_num(BUZZER_PIN);
    pwm_set_enabled(slice_num, true);
}

// Função para tocar o buzzer
void tocar_buzzer(int freq, int duration) {
    uint slice_num = pwm_gpio_to_slice_num(BUZZER_PIN);
    uint clock_freq = 125000000;
    uint16_t top = clock_freq / freq - 1;

    pwm_set_wrap(slice_num, top);
    pwm_set_gpio_level(BUZZER_PIN, top / 2);
    sleep_ms(duration);
    pwm_set_gpio_level(BUZZER_PIN, 0);
}

// Função para piscar o LED vermelho
void piscar_led_vermelho(int vezes) {
    uint32_t led_buffer[NUM_PIXELS] = {0};
    for (int i = 0; i < vezes; i++) {
        for (int j = 0; j < NUM_PIXELS; j++) {
            led_buffer[j] = 0xFF0000;
        }
        for (int j = 0; j < NUM_PIXELS; j++) {
            pio_sm_put_blocking(pio_global, sm_global, led_buffer[j]);
        }
        sleep_ms(500);
        for (int j = 0; j < NUM_PIXELS; j++) {
            led_buffer[j] = 0x000000;
        }
        for (int j = 0; j < NUM_PIXELS; j++) {
            pio_sm_put_blocking(pio_global, sm_global, led_buffer[j]);
        }
        sleep_ms(500);
    }
}

// Função para exibir um número na matriz de LEDs
void exibir_numero_na_matriz(int numero, PIO pio, uint sm, uint32_t *led_buffer) {
    if (numero >= 0 && numero <= 9) {
        for (int i = 0; i < NUM_PIXELS; i++) {
            led_buffer[i] = digitos[numero][i] ? 0xFFFFFF : 0x000000;
        }
        for (int i = 0; i < NUM_PIXELS; i++) {
            pio_sm_put_blocking(pio, sm, led_buffer[i]);
        }
    }
}

// Função para exibir uma mensagem no display OLED
void exibir_mensagem_no_display(ssd1306_t *ssd, const char *mensagem) {
    ssd1306_fill(ssd, false);
    ssd1306_draw_string(ssd, mensagem, 10, 30);
    ssd1306_send_data(ssd);
}

// Função para desligar a matriz de LEDs
void desligar_matriz_leds(PIO pio, uint sm) {
    for (int i = 0; i < NUM_PIXELS; i++) {
        pio_sm_put_blocking(pio, sm, 0x000000);
    }
}

// Função para atualizar o display com o tempo de cozimento
void atualizar_display_tempo() {
    char buffer[20];
    snprintf(buffer, sizeof(buffer), "Tempo: %02d:%02d", tempo_cozimento / 60, tempo_cozimento % 60);
    exibir_mensagem_no_display(&ssd, buffer);
}

// Callback para os botões
void gpio_callback(uint gpio, uint32_t events) {
    uint32_t current_time = to_ms_since_boot(get_absolute_time());

    if (gpio == BUTTON_A_PIN) {
        if (current_time - last_time_a < DEBOUNCE_DELAY) return;
        last_time_a = current_time;
        if (!microondas_ligado) {
            tempo_cozimento += 10;
            atualizar_display_tempo();
        }
    } else if (gpio == BUTTON_B_PIN) {
        if (current_time - last_time_b < DEBOUNCE_DELAY) return;
        last_time_b = current_time;
        if (!microondas_ligado) {
            potencia = (potencia == 800) ? 1200 : 800;
            char buffer[20];
            snprintf(buffer, sizeof(buffer), "Potencia: %dW", potencia);
            exibir_mensagem_no_display(&ssd, buffer);
            mover_servo(potencia == 800 ? DUTY_CYCLE_90 : DUTY_CYCLE_180);
        }
    } else if (gpio == BUTTON_JOY_PIN) {
        if (current_time - last_time_joy < DEBOUNCE_DELAY) return;
        last_time_joy = current_time;
        microondas_ligado = !microondas_ligado;
        if (microondas_ligado) {
            exibir_mensagem_no_display(&ssd, "Microondas Ligado");
            gpio_put(LED_G_GPIO, 1);
        } else {
            exibir_mensagem_no_display(&ssd, "Microondas Desligado");
            gpio_put(LED_G_GPIO, 0);
            tempo_cozimento = 0;
            atualizar_display_tempo();
        }
    }
}

// Função principal
int main() {
    stdio_init_all();
    sleep_ms(300);

    printf("RP2040 inicializado.\n");

    // Configuração dos botões
    gpio_init(BUTTON_A_PIN);
    gpio_set_dir(BUTTON_A_PIN, GPIO_IN);
    gpio_pull_up(BUTTON_A_PIN);

    gpio_init(BUTTON_B_PIN);
    gpio_set_dir(BUTTON_B_PIN, GPIO_IN);
    gpio_pull_up(BUTTON_B_PIN);

    gpio_init(BUTTON_JOY_PIN);
    gpio_set_dir(BUTTON_JOY_PIN, GPIO_IN);
    gpio_pull_up(BUTTON_JOY_PIN);

    // Configuração dos LEDs
    gpio_init(LED_G_GPIO);
    gpio_set_dir(LED_G_GPIO, GPIO_OUT);
    gpio_put(LED_G_GPIO, 0);

    gpio_init(LED_B_GPIO);
    gpio_set_dir(LED_B_GPIO, GPIO_OUT);
    gpio_put(LED_B_GPIO, 0);

    // Configuração do buzzer
    gpio_init(BUZZER_PIN);
    gpio_set_dir(BUZZER_PIN, GPIO_OUT);
    configurar_buzzer();

    // Configuração do servo motor
    configurar_servo();

    // Configuração do I2C e display OLED
    i2c_init(I2C_PORT, 400 * 1000);
    gpio_set_function(I2C_SDA, GPIO_FUNC_I2C);
    gpio_set_function(I2C_SCL, GPIO_FUNC_I2C);
    gpio_pull_up(I2C_SDA);
    gpio_pull_up(I2C_SCL);
    ssd1306_init(&ssd, 128, 64, false, endereco, I2C_PORT);
    ssd1306_config(&ssd);
    ssd1306_send_data(&ssd);
    ssd1306_fill(&ssd, false);
    ssd1306_send_data(&ssd);

    // Configuração da matriz de LEDs
    pio_global = pio0;
    sm_global = 0;
    uint offset = pio_add_program(pio_global, &ws2812_program);
    ws2812_program_init(pio_global, sm_global, offset, WS2812_PIN, 800000, false);

    uint32_t led_buffer[NUM_PIXELS] = {0};

    // Configuração das interrupções dos botões
    gpio_set_irq_enabled_with_callback(BUTTON_A_PIN, GPIO_IRQ_EDGE_FALL, true, gpio_callback);
    gpio_set_irq_enabled(BUTTON_B_PIN, GPIO_IRQ_EDGE_FALL, true);
    gpio_set_irq_enabled(BUTTON_JOY_PIN, GPIO_IRQ_EDGE_FALL, true);

    char received_char = '\0';
    bool cor = true;

    // Loop principal
    while (true) {
        cor = !cor;
        if (microondas_ligado && tempo_cozimento > 0) {
            sleep_ms(1000);
            tempo_cozimento--;
            atualizar_display_tempo();
            controlar_servo(tempo_cozimento); // Controla a velocidade do servo motor
            if (tempo_cozimento == 0) {
                microondas_ligado = false;
                ssd1306_rect(&ssd, 3, 3, 122, 58, true, false);
                exibir_mensagem_no_display(&ssd, "Cozimento concluido");
                gpio_put(LED_G_GPIO, 0);

                tocar_buzzer(1000, 500);
                piscar_led_vermelho(3);
            }
        }

        if (stdio_usb_connected()) {
            int ch = getchar_timeout_us(0);
            if (ch != PICO_ERROR_TIMEOUT) {
                received_char = (char) ch;
                printf("Recebido: %c\n", received_char);
                char mensagem[2] = {received_char, '\0'};
                exibir_mensagem_no_display(&ssd, mensagem);
                if (received_char >= '0' && received_char <= '9') {
                    int numero = received_char - '0';
                    exibir_numero_na_matriz(numero, pio_global, sm_global, led_buffer);
                }
            }
        }
        sleep_ms(100);
    }
}
