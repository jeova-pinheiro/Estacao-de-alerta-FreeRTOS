# Projeto: Estação de Alerta: Monitoramento Inteligente de Enchentes

**Fase 2 - EmbarcaTech**

## 📌 Descrição

Este projeto implementa uma **estação de monitoramento de enchentes** utilizando o microcontrolador **Raspberry Pi Pico** na placa **BitDogLab**, com o sistema operacional **FreeRTOS**. O sistema coleta dados em tempo real de **sensores de nível de água e volume de chuva**, e classifica a situação como **Seguro**, **Alerta** ou **Enchente**.
Os dados são apresentados em um **display OLED SSD1306**, a **matriz de LEDs WS2812** exibe uma animação conforme o nível da enchente, o **LED RGB** muda de cor com o estado atual, e o **buzzer** emite sinais sonoros distintos para cada situação.

## 🛠️ Componentes Utilizados

* **Microcontrolador:** Raspberry Pi Pico (Placa BitDogLab)
* **Sensores analógicos:** Nível da água e volume de chuva (via ADC)
* **Display OLED SSD1306** (via I2C, para exibição dos dados em tempo real)
* **Matriz de LEDs WS2812** (animação gráfica do nível de enchente, controlada via PIO)
* **LED RGB** (indicação visual do estado atual: seguro, alerta ou enchente)
* **Buzzer** (indicação sonora com padrões diferentes por estado)
* **Botão B (BOOTSEL)** (entrada digital para entrar em modo de reprogramação)

## 🔥 Funcionalidades

* ✅ Leitura contínua dos sensores de nível e chuva com conversão para porcentagem
* ✅ Cálculo do estado do sistema com base nos dados dos sensores
* ✅ Exibição das informações no display OLED
* ✅ Sinalização visual com LED RGB e animação em matriz de LEDs
* ✅ Alerta sonoro com buzzer (sem som para "seguro", bip longo para "alerta", bip rápido para "enchente")
* ✅ Modo BOOTSEL com botão físico para facilitar reprogramação

## 📄 Estrutura do Projeto

* `Principal.c` → Código principal com todas as tarefas do FreeRTOS
* `desenhosMatriz.h` / `desenhosMatriz.c` → Funções gráficas para a matriz WS2812
* `font.h` / `ssd1306.h` → Controle e renderização no display OLED
* `pio_matrix.pio` → Programa PIO para controle da matriz de LEDs WS2812
* `README.md` → Documentação do projeto

## 🖥️ Como Executar o Projeto

### Passo 1: Clone o repositório do projeto e abra-o no VS Code

### Passo 2: Configurar o Ambiente

Certifique-se de que o **Pico SDK** está corretamente instalado e que o **VS Code** possui a extensão **Raspberry Pi Pico**.

### Passo 3: Compilar o Código

Compile o projeto com a opção "Build" no VS Code.

### Passo 4: Carregar o Código na Placa

1. Conecte a placa **BitDogLab** via cabo USB
2. Pressione o **botão B (BOOTSEL)** e conecte o USB para entrar no modo de reprogramação
3. Use a opção "**Run Project (USB)**" da extensão para enviar o arquivo `.uf2`

### Passo 5: Observar o Funcionamento

* A cada segundo, os sensores atualizam os dados.
* O display OLED mostra os valores de **água**, **chuva** e o **estado atual**.
* O LED RGB muda de cor:

  * Verde para **Seguro**
  * Amarelo para **Alerta**
  * Vermelho para **Enchente**
* O buzzer emite sons conforme o estado:

  * Silêncio em **Seguro**
  * Bip longo e espaçado em **Alerta**
  * Bip rápido em **Enchente**
* A matriz WS2812 exibe animações conforme o nível de água.

## 📌 Autor

Projeto desenvolvido por **Jeová Pinheiro** para a fase 2 do ***EmbarcaTech***.
