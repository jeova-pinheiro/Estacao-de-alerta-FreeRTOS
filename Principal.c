#include "pico/stdlib.h"
#include <stdio.h>
#include "hardware/gpio.h"
#include "hardware/adc.h"
#include "hardware/i2c.h"
#include "hardware/pwm.h"
#include "pico/bootrom.h"
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "lib/ssd1306.h"
#include "lib/font.h"
#include "lib/desenhosMatriz.h"

// === Definições dos pinos ===
#define I2C_PORT i2c1
#define I2C_SDA 14
#define I2C_SCL 15
#define ENDERECO_OLED 0x3C

// Pinos do Joystic para ADC
#define ADC_CHUVA 26
#define ADC_AGUA 27

// Pinos do LED RGB
#define LED_R 13
#define LED_G 11
#define LED_B 12

// Pino para matriz de LEDs
#define MATRIZ_WS2812B 7

// Pino para o BUZZER
#define BUZZER 21

// Pino do botão B para BOOTSEL
#define BOTAO_B 6

// Varivaveis para regular níveis de chuva e água
uint16_t chuva = 0;
uint16_t agua = 0;

// Definição dos estados do sistema
#define SEGURO 0
#define ALERTA 1
#define ENCHENTE 2

int estado_sistema = SEGURO;

// Fila usada para transmitir os dados dos sensores (nível de água e chuva) entre tarefas
QueueHandle_t xQueueSensorData;

// Inicializa ADC para Joystick
void init_adc()
{
    adc_init();
    adc_gpio_init(ADC_AGUA);
    adc_gpio_init(ADC_CHUVA);
}

// Inicializa pinos para o display OLED
void init_i2c_display(ssd1306_t *ssd)
{
    // Inicializa a comunicação I2C
    i2c_init(I2C_PORT, 400000);
    gpio_set_function(I2C_SDA, GPIO_FUNC_I2C);
    gpio_set_function(I2C_SCL, GPIO_FUNC_I2C);
    gpio_pull_up(I2C_SDA);
    gpio_pull_up(I2C_SCL);

    // Inicializa o display OLED
    ssd1306_init(ssd, 128, 64, false, ENDERECO_OLED, I2C_PORT);
    ssd1306_config(ssd);
    ssd1306_fill(ssd, false);
    ssd1306_send_data(ssd);
}

// Inicializa PWM para led RGB
void init_rgb_pwm(uint *slice_r, uint *slice_g, uint *slice_b, uint *chan_r, uint *chan_g, uint *chan_b)
{
    gpio_set_function(LED_R, GPIO_FUNC_PWM);
    gpio_set_function(LED_G, GPIO_FUNC_PWM);
    gpio_set_function(LED_B, GPIO_FUNC_PWM);

    *slice_r = pwm_gpio_to_slice_num(LED_R);
    *slice_g = pwm_gpio_to_slice_num(LED_G);
    *slice_b = pwm_gpio_to_slice_num(LED_B);

    *chan_r = pwm_gpio_to_channel(LED_R);
    *chan_g = pwm_gpio_to_channel(LED_G);
    *chan_b = pwm_gpio_to_channel(LED_B);

    pwm_set_clkdiv(*slice_r, 100.0f);
    pwm_set_clkdiv(*slice_g, 100.0f);
    pwm_set_clkdiv(*slice_b, 100.0f);
    pwm_set_wrap(*slice_r, 255);
    pwm_set_wrap(*slice_g, 255);
    pwm_set_wrap(*slice_b, 255);

    pwm_set_chan_level(*slice_r, *chan_r, 0);
    pwm_set_chan_level(*slice_g, *chan_g, 0);
    pwm_set_chan_level(*slice_b, *chan_b, 0);

    pwm_set_enabled(*slice_r, true);
    pwm_set_enabled(*slice_g, true);
    pwm_set_enabled(*slice_b, true);
}

// Inializa pinos para PWM
void init_buzzer_pwm()
{
    gpio_set_function(BUZZER, GPIO_FUNC_PWM);
    uint slice = pwm_gpio_to_slice_num(BUZZER);
    pwm_set_clkdiv(slice, 100.0f);
    pwm_set_wrap(slice, 255);
    pwm_set_chan_level(slice, pwm_gpio_to_channel(BUZZER), 0);
    pwm_set_enabled(slice, true);
}

// Inicializa pino para matriz de led
void init_matriz()
{
    npInit(MATRIZ_WS2812B);
    srand(to_ms_since_boot(get_absolute_time())); // Inicializa somente para números aleatórios
}

/* === Tarefas === */

// Tarefa para controle do ADC do Joystick
void vSensorADCTask(void *params)
{
    init_adc();

    while (true)
    {
        adc_select_input(0);
        agua = adc_read();

        adc_select_input(1);
        chuva = adc_read();

        // Sinaliza atualização na fila (pode enviar NULL ou algum valor dummy)
        uint8_t sinal = 1; // dado dummy só para notificar a atualização
        xQueueSend(xQueueSensorData, &sinal, 0);

        vTaskDelay(pdMS_TO_TICKS(100));
    }
}

// Tarefa para display Oled
void vDisplayOledTaks(void *params)
{
    ssd1306_t ssd;
    init_i2c_display(&ssd);
    char buffer[32];
    const char *status;
    uint8_t sinal;

    while (true)
    {
        // Espera só por sinal de atualização (sem dados na fila)
        if (xQueueReceive(xQueueSensorData, &sinal, portMAX_DELAY))
        {
            // Lê as variáveis globais
            uint8_t agua_perc = (agua * 100) / 4095;
            uint8_t chuva_perc = (chuva * 100) / 4095;

            ssd1306_fill(&ssd, false);

            if (agua_perc >= 70 || chuva_perc >= 80)
            {
                status = "Enchente";
                estado_sistema = ENCHENTE;
            }
            else if (agua_perc >= 50 || chuva_perc >= 50)
            {
                status = "Alerta";
                estado_sistema = ALERTA;
            }
            else
            {
                status = "Seguro";
                estado_sistema = SEGURO;
            }

            snprintf(buffer, sizeof(buffer), "Agua: %d%%", agua_perc);
            ssd1306_draw_string(&ssd, buffer, 25, 4);
            snprintf(buffer, sizeof(buffer), "Chuva: %d%%", chuva_perc);
            ssd1306_draw_string(&ssd, buffer, 25, 15);
            snprintf(buffer, sizeof(buffer), "%s", status);
            ssd1306_draw_string(&ssd, buffer, 35, 30);

            uint8_t largura = agua_perc;
            ssd1306_rect(&ssd, 48, 15, largura, 8, true, true);
            ssd1306_rect(&ssd, 48, 15, 100, 8, true, false);
            ssd1306_send_data(&ssd);
        }
    }
}

// Tarefa para led RGB
void vLedRgbTask(void *params)
{
    uint slice_r, slice_g, slice_b;
    uint chan_r, chan_g, chan_b;
    init_rgb_pwm(&slice_r, &slice_g, &slice_b, &chan_r, &chan_g, &chan_b);

    while (1)
    {
        switch (estado_sistema)
        {
        case SEGURO:
            pwm_set_chan_level(slice_r, chan_r, 0);
            pwm_set_chan_level(slice_g, chan_g, 255);
            pwm_set_chan_level(slice_b, chan_b, 0);
            break;
        case ALERTA:
            pwm_set_chan_level(slice_r, chan_r, 255);
            pwm_set_chan_level(slice_g, chan_g, 128);
            pwm_set_chan_level(slice_b, chan_b, 0);
            break;
        case ENCHENTE:
            pwm_set_chan_level(slice_r, chan_r, 255);
            pwm_set_chan_level(slice_g, chan_g, 0);
            pwm_set_chan_level(slice_b, chan_b, 0);
            break;
        }
        vTaskDelay(pdMS_TO_TICKS(200));
    }
}

// Tarefa para Buzzer
void vBuzzerTask(void *params)
{
    init_buzzer_pwm();
    uint slice = pwm_gpio_to_slice_num(BUZZER);
    uint chan = pwm_gpio_to_channel(BUZZER);

    while (1)
    {
        switch (estado_sistema)
        {
        case ENCHENTE:
            // Bip curto e rápido
            pwm_set_chan_level(slice, chan, 128);
            vTaskDelay(pdMS_TO_TICKS(200));
            pwm_set_chan_level(slice, chan, 0);
            vTaskDelay(pdMS_TO_TICKS(200));
            break;

        case ALERTA:
            // Bip longo e mais espaçado
            pwm_set_chan_level(slice, chan, 128);
            vTaskDelay(pdMS_TO_TICKS(1000));
            pwm_set_chan_level(slice, chan, 0);
            vTaskDelay(pdMS_TO_TICKS(2000));
            break;

        case SEGURO:
        default:
            // Silêncio
            pwm_set_chan_level(slice, chan, 0);
            vTaskDelay(pdMS_TO_TICKS(1000));
            break;
        }
    }
}

// Tarefa para matriz de led
void vMatrizLedTask(void *params)
{
    init_matriz();

    while (true)
    {
        // Recebe dados da fila (bloqueia até receber)
        if (xQueueReceive(xQueueSensorData, NULL, portMAX_DELAY) == pdTRUE)
        {
            // Agora os dados estão nas variáveis globais 'agua' e 'chuva'

            // Converte valores brutos para percentuais (usado apenas para depuração)
            uint8_t nivel_agua = (agua * 100) / 4095;    // Nível de água
            uint8_t volume_chuva = (chuva * 100) / 4095; // Volume de chuva

            // Executa animação de enchente com base no nível de água
            desenhoMatriz(agua); // Passa valor bruto para a função
        }
    }
}

/* === Interrupção do Botão B === */
void gpio_irq_handler(uint gpio, uint32_t events)
{
    if (gpio == BOTAO_B && (events & GPIO_IRQ_EDGE_FALL))
        reset_usb_boot(0, 0);
}

/* === Função Principal (Main) === */
int main()
{
    stdio_init_all();

    // Configura o botão B (BOOTSEL)
    gpio_init(BOTAO_B);             // Inicializa GPIO6
    gpio_set_dir(BOTAO_B, GPIO_IN); // Configura como entrada
    gpio_pull_up(BOTAO_B);          // Ativa pull-up interno

    gpio_set_irq_enabled_with_callback(BOTAO_B, GPIO_IRQ_EDGE_FALL, true, &gpio_irq_handler);

    // Cria fila para envio de sinais simples entre tarefas, capacidade para 5 mensagens uint8_t
    xQueueSensorData = xQueueCreate(5, sizeof(uint8_t));

    // Cria tarefas do FreeRTOS
    xTaskCreate(vSensorADCTask, "Sensor ADC Task", 256, NULL, 1, NULL);     // Tarefa de sensores (Joystick)
    xTaskCreate(vDisplayOledTaks, "Display Oled Task", 512, NULL, 2, NULL); // Tarefa do display OLED
    xTaskCreate(vLedRgbTask, "LED RGB Task", 256, NULL, 2, NULL);           // Tarefa do LED RGB
    xTaskCreate(vBuzzerTask, "Buzzer Task", 256, NULL, 2, NULL);            // Tarefa do buzzer
    xTaskCreate(vMatrizLedTask, "Matriz de LED Task", 256, NULL, 2, NULL);  // Tarefa da matriz de LED

    vTaskStartScheduler();
    panic_unsupported(); // Caso o escalonador falhe
}
