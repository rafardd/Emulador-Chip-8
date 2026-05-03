#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdbool.h>
#include "functions.c"

typedef struct
{
	uint8_t memory[4096]; // Os primeiros 512 eram onde ficava o interpretador e não serão usados
	uint8_t V[0xF];
	uint16_t i;	 // Registrador de 16 bits que costuma guardar endereços, então costuma usar somente 12 bits
	uint16_t pc; // Program counter
	uint8_t sp;	 // Stack pointer
	uint16_t stack[16];
	uint8_t DT;			  // Timer register
	uint8_t ST;			  // Sound timer
	uint8_t gfx[64 * 32]; // Pixels da tela
	uint8_t keypad[16];	  // Estado do teclado
} Chip8_struct;

Chip8_struct Chip8;

void initValues()
{
	Chip8.pc = 200;
	Chip8.sp = 0;
}
// Operações com a stack

void insertStack()
{
	Chip8.sp++;
	Chip8.stack[Chip8.sp] == Chip8.pc;
}
void removeStack()
{
	Chip8.pc = Chip8.stack[Chip8.sp];
	Chip8.sp--;
}

int main()
{
	FILE *ROM = fopen("Pong [Paul Verevalin, 1990].ch8", "r");
	if (ROM == NULL)
	{
		printf("Erro ao abrir o arquivo\n");
		return 0;
	}

	fgets(Chip8.memory, 4096, ROM);
	// Carregando conteúdo da ROM para memória do chip
	fclose(ROM);

	// Inicialização de valores
	initValues();

	bool running = true;
	while (running)
	{

		uint16_t opcode = (readNext(Chip8.pc) << 8) | readNext(Chip8.pc + 1);
		switch (opcode & 0xF000)
		{
		case 0x0000:
			if (opcode & 0x0FFF == 0x0E0)
			{
				// clear display
				break;
			}
			// 0x0EE
			removeStack();
			break;
		case 0x1000:
		{
			uint16_t address = opcode & 0x0FFF;
			Chip8.pc = address;
			break;
		}

		case 0x2000:
		{
			insertStack();
			uint16_t address = opcode & 0x0FFF;
			Chip8.pc = address;
			break;
		}

		case 0x3000:
		{
			uint8_t x = (opcode & 0x0F00) >> 8;
			uint8_t kk = opcode & 0x00FF;
			if (Chip8.V[x] == kk)
			{
				Chip8.pc += 2;
			}
			break;
		}
		case 0x4000:
		{
			uint8_t x = (opcode & 0x0F00) >> 8;
			uint8_t kk = opcode & 0x00FF;
			if (Chip8.V[x] != kk)
			{
				Chip8.pc += 2;
			}
			break;
		}
		case 0x5000:
		{
			uint8_t x = (opcode & 0x0F00) >> 8;
			uint8_t y = (opcode & 0x00F0) >> 4;
			if (Chip8.V[x] == Chip8.V[y])
			{
				Chip8.pc += 2;
			}
			break;
		}
		case 0x6000:
		{
			uint8_t x = (opcode & 0x0F00) >> 8;
			uint8_t kk = opcode & 0x00FF;
			Chip8.V[x] = kk;
			break;
		}
		case 0x7000:
		{
			uint8_t x = (opcode & 0x0F00) >> 8;
			uint8_t kk = opcode & 0x00FF;
			Chip8.V[x] += kk;
			break;
		}
		case 0x8000:
		{
			uint8_t x = (opcode & 0x0F00) >> 8;
			uint8_t y = (opcode & 0x00F0) >> 4;
			switch (opcode & 0x000F)
			{
			case 0x0:
				Chip8.V[x] = Chip8.V[y];
				break;
			case 0x1:
				Chip8.V[x] = Chip8.V[x] | Chip8.V[y];
				break;
			case 0x2:
				Chip8.V[x] = Chip8.V[x] & Chip8.V[y];
				break;
			case 0x3:
				Chip8.V[x] = Chip8.V[x] ^ Chip8.V[y];
				break;
			case 0x4:
			{
				uint16_t result = Chip8.V[x] + Chip8.V[y];
				if (result > 255)
				{
					Chip8.V[0xF] = 1;
				}
				else
				{
					Chip8.V[0xF] = 0;
				}
				Chip8.V[x] = result & 0x00FF;
				break;
			}
			case 0x5:
			{
				uint16_t result = Chip8.V[x] - Chip8.V[y];
				if (Chip8.V[x] > Chip8.V[y])
				{
					Chip8.V[0xF] = 1;
				}
				else
				{
					Chip8.V[0xF] = 0;
				}
				Chip8.V[x] = result;
				break;
			}
			case 0x6:
			{
				if (Chip8.V[x] & 1 == 1)
				{
					Chip8.V[0xF] = 1;
				}
				else
				{
					Chip8.V[0xF] = 0;
				}
				Chip8.V[x] /= 2;
				break;
			}
			case 0x7:
				if (Chip8.V[y] > Chip8.V[x])
				{
					Chip8.V[0xF] = 1;
				}
				else
				{
					Chip8.V[0xF] = 0;
				}
				Chip8.V[x] = Chip8.V[y] - Chip8.V[x];
				break;
			case 0xE:
				if (Chip8.V[x] & 128 == 1)
				{
					Chip8.V[0xF] = 1;
				}
				else
				{
					Chip8.V[0xF] = 0;
				}
				Chip8.V[x] *= 2;
				break;
			}
			break;
		}
		case 0x9000:
		}
	}
