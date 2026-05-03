#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdbool.h>
#include "functions.c"
typedef struct{
    uint8_t memory[4096]; // Os primeiros 512 eram onde ficava o interpretador e não serão usados
    uint8_t V[16];
    uint16_t i; // Registrador de 16 bits que costuma guardar endereços, então costuma usar somente 12 bits
    uint16_t pc; // Program counter
    uint8_t sp; // Stack pointer
    uint16_t stack[16];
    uint8_t DT; // Timer register
    uint8_t ST; // Sound timer
    uint8_t gfx[64 * 32];  // Pixels da tela
    uint8_t keypad[16];   // Estado do teclado
} Chip8_struct;

Chip8_struct Chip8;

void initValues(){
    Chip8.pc = 200;
}

    
    

int main(){
    FILE *ROM = fopen("Pong [Paul Verevalin, 1990].ch8","r");
    if(ROM == NULL){
	printf("Erro ao abrir o arquivo\n");
	return 0;
    }
    

    fgets(Chip8.memory,4096,ROM);
    // Carregando conteúdo da ROM para memória do chip
    fclose(ROM);

    // Inicialização de valores
    initValues();

    bool running = true;
    while(running){

	uint16_t opcode = (memory[pc] << 8) | memory[pc + 1];
	switch(opcode  & xF000){
	case x00E0:
	    //clear display
	    break;
	case x00EE:
	    //return from subroutine
	    break;
	case x1:
	    Chip8.pc = nextAddress
	    
	
	
    
}

    
    
