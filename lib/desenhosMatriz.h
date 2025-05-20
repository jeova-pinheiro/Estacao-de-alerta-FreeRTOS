#include "matrizled.c"
#include "FreeRTOS.h"
#include "task.h"
#include <time.h>
#include <stdlib.h>

// Intensidade padrão para as animações
#define intensidade 1

void printNum(void)
{
    npWrite();
    npClear();
}

// Animação para ALERTA: chuva piscante azul
void anim_alerta(void)
{
    npClear();
    for (int i = 0; i < 3; i++)
    {
        int x = rand() % 5;
        int y = rand() % 5;
        int posicao = getIndex(x, y);
        npSetLED(posicao, 0, 0, 100); // Azul
    }
    npWrite();
    vTaskDelay(pdMS_TO_TICKS(500)); // 2 Hz
}

// Desenho na matriz de led para ENCHENTE
void desenhoMatriz(uint16_t nivel_agua)
{
    npClear();

    // Converte nível de água para percentual (0–100)
    uint8_t percent_agua = (nivel_agua * 100) / 4095;

    // Verifica intervalos e acende linhas horizontais em azul (de baixo para cima)
    if (percent_agua >= 98)
    {
        // Linhas 4, 3, 2, 1, 0 (todas)
        for (int row = 0; row < 5; row++)
        {
            for (int col = 0; col < 5; col++)
            {
                int posicao = getIndex(col, row);
                npSetLED(posicao, 100, 0, 0); // Vermelho
            }
        }
    }
    else if (percent_agua >= 80)
    {
        // Linhas 4, 3, 2, 1
        for (int row = 1; row <= 4; row++)
        {
            for (int col = 0; col < 5; col++)
            {
                int posicao = getIndex(col, row);
                npSetLED(posicao, 100, 0, 0); // Vermelho
            }
        }
    }
    else if (percent_agua >= 60)
    {
        // Linhas 4, 3, 2
        for (int row = 2; row <= 4; row++)
        {
            for (int col = 0; col < 5; col++)
            {
                int posicao = getIndex(col, row);
                npSetLED(posicao, 100, 100, 0); // Amarelo
            }
        }
    }
    else if (percent_agua >= 40)
    {
        // Linhas 4, 3
        for (int row = 3; row <= 4; row++)
        {
            for (int col = 0; col < 5; col++)
            {
                int posicao = getIndex(col, row);
                npSetLED(posicao, 100, 100, 0); // Amarelo
            }
        }
    }
    else if (percent_agua >= 20)
    {
        // Linha 4
        for (int col = 0; col < 5; col++)
        {
            int posicao = getIndex(col, 4);
            npSetLED(posicao, 0, 100, 0); // Verde
        }
    }
    // 0–19%: Nenhuma linha (npClear já limpou)

    npWrite();
    vTaskDelay(pdMS_TO_TICKS(100)); // 10 Hz
}
