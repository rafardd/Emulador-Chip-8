
# Emulador CHIP-8 em C

Um interpretador completo da máquina virtual CHIP-8 escrito do zero em C e SDL2.

Este projeto foi construído com o objetivo de aprofundar conhecimentos em arquitetura de computadores, manipulação de memória (baixo nível), operações bitwise e ciclo de execução de CPU (Fetch, Decode, Execute). Foi feito em um final de semana utilizando somente documentação e leves revisões com uma LLM.
<img width="800" height="460" alt="demo" src="https://github.com/user-attachments/assets/2e4be181-3064-4d26-9339-ec0913697eb8" />

## Funcionalidades Implementadas

- **Ciclo de CPU Completo:** Implementação fiel dos 35 opcodes originais do CHIP-8.
- **Gráficos via SDL2:** Renderização da matriz monocromática de 64x32 pixels com fator de escala ajustável.
- **Controles Mapeados:** Suporte ao teclado hexadecimal original mapeado para o teclado moderno (layout QWERTY/AZERTY).
- **Timers Independentes:** _Delay Timer_ e _Sound Timer_ rodando precisamente a 60Hz.
- **Áudio Nativo:** Geração de onda quadrada (beep) utilizando a API de áudio do SDL2 sem depender de arquivos externos.
- **Clock Dinâmico:** Capacidade de ajustar a velocidade de execução da CPU via linha de comando para suportar diferentes tipos de jogos.

## Controles

O teclado original do CHIP-8 possuía 16 teclas hexadecimais (0-F). Elas foram mapeadas para o lado esquerdo do teclado moderno da seguinte forma:

| CHIP-8  | Teclado Moderno |
| :-----: | :-------------: |
| 1 2 3 C |     1 2 3 4     |
| 4 5 6 D |     Q W E R     |
| 7 8 9 E |     A S D F     |
| A 0 B F |     Z X C V     |

## Como Compilar e Rodar

### 1. Pré-requisitos

Para compilar o emulador, você precisará de um compilador C (`gcc`) e da biblioteca de desenvolvimento do SDL2. Instale de acordo com a sua distribuição Linux.

### 2. Compilando

Clone este repositório e compile o código fonte utilizando o comando abaixo ou faça o donwload pela aba de releases:

```bash
gcc main.c -o chip8 -lSDL2
```

### 3. Executando

O emulador recebe a ROM do jogo como argumento obrigatório e a velocidade do clock (instruções por frame) como argumento opcional. A velocidade padrão é 10.

```bash

# Rodando com a velocidade padrão (~600Hz)
./chip8 roms/Pong.ch8

# Rodando um jogo mais rápido exigindo 20 instruções por frame (~1200Hz)
./chip8 roms/SpaceInvaders.ch8 20
```

## Referências e Materiais de Estudo

- [Cowgod's CHIP-8 Technical Reference](http://devernay.free.fr/hacks/chip8/C8TECH10.HTM) - O manual definitivo dos opcodes e arquitetura.
