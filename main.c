#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdbool.h>
#include <time.h>
#include <SDL2/SDL.h>
// Constantes da janela
#define VIDEO_WIDTH 64
#define VIDEO_HEIGHT 32
#define SCALE 15 // A janela terá 960x480 pixels

typedef struct
{
	uint8_t memory[4096]; // Os primeiros 512 eram onde ficava o interpretador e não serão usados
	uint8_t V[0xF];
	uint16_t I;	 // Registrador de 16 bits que costuma guardar endereços, então costuma usar somente 12 bits
	uint16_t PC; // Program counter
	uint8_t SP;	 // Stack pointer
	uint16_t stack[16];
	uint8_t DT;			  // Timer register
	uint8_t ST;			  // Sound timer
	uint8_t gfx[64 * 32]; // Pixels da tela
	uint8_t keypad[16];	  // Estado do teclado
} Chip8_struct;

Chip8_struct Chip8;

// Áudio
const int SAMPLE_RATE = 44100; // Taxa de amostragem padrão de CDs
const int AMPLITUDE = 2000;	   // Volume do beep
int audio_phase = 0;

void audio_callback(void *userdata, uint8_t *stream, int len)
{
	int16_t *audio_data = (int16_t *)stream;
	int samples = len / 2;
	int half_period = SAMPLE_RATE / 440 / 2;

	for (int i = 0; i < samples; i++)
	{
		if ((audio_phase / half_period) % 2 == 0)
		{
			audio_data[i] = AMPLITUDE;
		}
		else
		{
			audio_data[i] = -AMPLITUDE;
		}
		audio_phase++;
	}
}

// Fontset
uint8_t fontset[80] = {
	0xF0, 0x90, 0x90, 0x90, 0xF0, // 0
	0x20, 0x60, 0x20, 0x20, 0x70, // 1
	0xF0, 0x10, 0xF0, 0x80, 0xF0, // 2
	0xF0, 0x10, 0xF0, 0x10, 0xF0, // 3
	0x90, 0x90, 0xF0, 0x10, 0x10, // 4
	0xF0, 0x80, 0xF0, 0x10, 0xF0, // 5
	0xF0, 0x80, 0xF0, 0x90, 0xF0, // 6
	0xF0, 0x10, 0x20, 0x40, 0x40, // 7
	0xF0, 0x90, 0xF0, 0x90, 0xF0, // 8
	0xF0, 0x90, 0xF0, 0x10, 0xF0, // 9
	0xF0, 0x90, 0xF0, 0x90, 0x90, // A
	0xE0, 0x90, 0xE0, 0x90, 0xE0, // B
	0xF0, 0x80, 0x80, 0x80, 0xF0, // C
	0xE0, 0x90, 0x90, 0x90, 0xE0, // D
	0xF0, 0x80, 0xF0, 0x80, 0xF0, // E
	0xF0, 0x80, 0xF0, 0x80, 0x80  // F
};

void loadFontSet()
{
	for (int i = 0; i < 80; ++i)
	{
		Chip8.memory[0x050 + i] = fontset[i];
	}
}
uint8_t readNext(uint16_t address)
{
	return Chip8.memory[address];
};
// Operações com janela

void clearScreen(SDL_Renderer *renderer)
{
	SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
	SDL_RenderClear(renderer);
}
void drawScreen(SDL_Renderer *renderer)
{
	SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
	for (int y = 0; y < VIDEO_HEIGHT; y++)
	{
		for (int x = 0; x < VIDEO_WIDTH; x++)
		{
			if (Chip8.gfx[x + (y * VIDEO_WIDTH)] == 1)
			{
				SDL_Rect pixelRect;
				pixelRect.x = x * SCALE;
				pixelRect.y = y * SCALE;
				pixelRect.w = SCALE;
				pixelRect.h = SCALE;
				SDL_RenderFillRect(renderer, &pixelRect);
			}
		}
	}
	SDL_RenderPresent(renderer);
	SDL_Delay(16); // Framerate
}
void quitScreen(SDL_Renderer *renderer, SDL_Window *window, SDL_AudioDeviceID *audio_device)
{
	SDL_DestroyRenderer(renderer);
	SDL_DestroyWindow(window);
	SDL_CloseAudioDevice(*audio_device);
	SDL_Quit();
}

// Inicialização de valores
void initValues()
{
	Chip8.PC = 0x200;
	Chip8.SP = 0;
}
// Operações com a stack

void insertStack()
{
	Chip8.SP++;
	Chip8.stack[Chip8.SP] = Chip8.PC;
}
void removeStack()
{
	Chip8.PC = Chip8.stack[Chip8.SP];
	Chip8.SP--;
}
// Operaçoes com o teclado
bool isPressed(uint8_t value)
{
	if (Chip8.keypad[value] == 1)
	{
		return true;
	}
	return false;
}

int main(int argc, char *argv[])
{
	if (argc < 2)
	{
		printf("Usage: %s <ROM file> [speed]\n", argv[0]);
		return 0;
	}
	int cycles_per_frame = 10;
	if (argc == 3)
	{
		cycles_per_frame = atoi(argv[2]);
	}
	FILE *ROM = fopen(argv[1], "rb");
	if (ROM == NULL)
	{
		printf("Erro ao abrir o arquivo\n");
		return 0;
	}

	fread(Chip8.memory + 0x200, 1, 4096 - 0x200, ROM);
	// Carregando conteúdo da ROM para memória do chip
	fclose(ROM);

	// Inicialização de valores
	initValues();
	loadFontSet();

	// Inicialização da janela e áudio
	SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO);
	SDL_Window *window = SDL_CreateWindow("CHIP-8 Emulator",
										  SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
										  VIDEO_WIDTH * SCALE, VIDEO_HEIGHT * SCALE, 0);
	SDL_Renderer *renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
	SDL_AudioSpec want, have;
	SDL_zero(want);
	want.freq = SAMPLE_RATE;
	want.format = AUDIO_S16SYS;
	want.channels = 1;
	want.samples = 2048;
	want.callback = audio_callback;

	SDL_AudioDeviceID audio_device = SDL_OpenAudioDevice(NULL, 0, &want, &have, 0);
	if (audio_device == 0)
	{
		printf("Aviso: Falha ao inicializar o áudio: %s\n", SDL_GetError());
	}
	bool running = true;
	srand(time(NULL));
	SDL_Event event;
	while (running)
	{
		while (SDL_PollEvent(&event))
		{
			if (event.type == SDL_QUIT)
			{
				running = false;
			}
			// Detecta quando uma tecla é PRESSIONADA
			else if (event.type == SDL_KEYDOWN)
			{
				switch (event.key.keysym.sym)
				{
				case SDLK_1:
					Chip8.keypad[0x1] = 1;
					break;
				case SDLK_2:
					Chip8.keypad[0x2] = 1;
					break;
				case SDLK_3:
					Chip8.keypad[0x3] = 1;
					break;
				case SDLK_4:
					Chip8.keypad[0xC] = 1;
					break;

				case SDLK_q:
					Chip8.keypad[0x4] = 1;
					break;
				case SDLK_w:
					Chip8.keypad[0x5] = 1;
					break;
				case SDLK_e:
					Chip8.keypad[0x6] = 1;
					break;
				case SDLK_r:
					Chip8.keypad[0xD] = 1;
					break;

				case SDLK_a:
					Chip8.keypad[0x7] = 1;
					break;
				case SDLK_s:
					Chip8.keypad[0x8] = 1;
					break;
				case SDLK_d:
					Chip8.keypad[0x9] = 1;
					break;
				case SDLK_f:
					Chip8.keypad[0xE] = 1;
					break;

				case SDLK_z:
					Chip8.keypad[0xA] = 1;
					break;
				case SDLK_x:
					Chip8.keypad[0x0] = 1;
					break;
				case SDLK_c:
					Chip8.keypad[0xB] = 1;
					break;
				case SDLK_v:
					Chip8.keypad[0xF] = 1;
					break;
				}
			}
			// Detecta quando uma tecla é SOLTA
			else if (event.type == SDL_KEYUP)
			{
				switch (event.key.keysym.sym)
				{
				case SDLK_1:
					Chip8.keypad[0x1] = 0;
					break;
				case SDLK_2:
					Chip8.keypad[0x2] = 0;
					break;
				case SDLK_3:
					Chip8.keypad[0x3] = 0;
					break;
				case SDLK_4:
					Chip8.keypad[0xC] = 0;
					break;

				case SDLK_q:
					Chip8.keypad[0x4] = 0;
					break;
				case SDLK_w:
					Chip8.keypad[0x5] = 0;
					break;
				case SDLK_e:
					Chip8.keypad[0x6] = 0;
					break;
				case SDLK_r:
					Chip8.keypad[0xD] = 0;
					break;

				case SDLK_a:
					Chip8.keypad[0x7] = 0;
					break;
				case SDLK_s:
					Chip8.keypad[0x8] = 0;
					break;
				case SDLK_d:
					Chip8.keypad[0x9] = 0;
					break;
				case SDLK_f:
					Chip8.keypad[0xE] = 0;
					break;

				case SDLK_z:
					Chip8.keypad[0xA] = 0;
					break;
				case SDLK_x:
					Chip8.keypad[0x0] = 0;
					break;
				case SDLK_c:
					Chip8.keypad[0xB] = 0;
					break;
				case SDLK_v:
					Chip8.keypad[0xF] = 0;
					break;
				}
			}
		}
		for (int z = 1; z <= cycles_per_frame; z++)
		{
			uint16_t opcode = (readNext(Chip8.PC) << 8) | readNext(Chip8.PC + 1);
			Chip8.PC += 2;
			switch (opcode & 0xF000)
			{
			case 0x0000:
				if ((opcode & 0x0FFF) == 0x0E0)
				{
					for (int i = 0; i < 2048; i++)
					{
						Chip8.gfx[i] = 0;
					}
					break;
				}
				// 0x0EE
				removeStack();
				break;
			case 0x1000:
			{
				uint16_t address = opcode & 0x0FFF;
				Chip8.PC = address;
				break;
			}

			case 0x2000:
			{
				insertStack();
				uint16_t address = opcode & 0x0FFF;
				Chip8.PC = address;
				break;
			}

			case 0x3000:
			{
				uint8_t x = (opcode & 0x0F00) >> 8;
				uint8_t kk = opcode & 0x00FF;
				if (Chip8.V[x] == kk)
				{
					Chip8.PC += 2;
				}
				break;
			}
			case 0x4000:
			{
				uint8_t x = (opcode & 0x0F00) >> 8;
				uint8_t kk = opcode & 0x00FF;
				if (Chip8.V[x] != kk)
				{
					Chip8.PC += 2;
				}
				break;
			}
			case 0x5000:
			{
				uint8_t x = (opcode & 0x0F00) >> 8;
				uint8_t y = (opcode & 0x00F0) >> 4;
				if (Chip8.V[x] == Chip8.V[y])
				{
					Chip8.PC += 2;
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
					if (Chip8.V[x] >= Chip8.V[y])
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
					if ((Chip8.V[x] & 1) == 1)
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
					if (Chip8.V[y] >= Chip8.V[x])
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
					if ((Chip8.V[x] & 128) != 0)
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
			{
				uint8_t x = (opcode & 0x0F00) >> 8;
				uint8_t y = (opcode & 0x00F0) >> 4;
				if (Chip8.V[x] != Chip8.V[y])
				{
					Chip8.PC += 2;
				}
				break;
			}
			case 0xA000:
			{
				uint16_t address = opcode & 0x0FFF;
				Chip8.I = address;
				break;
			}
			case 0xB000:
			{
				uint16_t address = opcode & 0x0FFF;
				Chip8.PC = address + Chip8.V[0];
				break;
			}
			case 0xC000:
			{
				uint8_t r = rand() % 256;
				uint8_t x = (opcode & 0x0F00) >> 8;
				uint8_t kk = opcode & 0x00FF;
				Chip8.V[x] = r & kk;
				break;
			}
			case 0xD000:
			{
				uint8_t x = (opcode & 0x0F00) >> 8;
				uint8_t y = (opcode & 0x00F0) >> 4;
				uint8_t height = opcode & 0x000F;
				uint8_t x_pos = Chip8.V[x] % 64;
				uint8_t y_pos = Chip8.V[y] % 32;
				Chip8.V[0xF] = 0;
				for (int row = 0; row < height; row++)
				{
					uint8_t sprite_byte = Chip8.memory[Chip8.I + row];

					for (int col = 0; col < 8; col++)
					{

						if ((sprite_byte & (0x80 >> col)) != 0)
						{

							int pixel_x = x_pos + col;
							int pixel_y = y_pos + row;

							if (pixel_x >= 64 || pixel_y >= 32)
							{
								continue;
							}

							int pixel_index = pixel_x + (pixel_y * 64);

							if (Chip8.gfx[pixel_index] == 1)
							{
								Chip8.V[0xF] = 1;
							}

							Chip8.gfx[pixel_index] ^= 1;
						}
					}
				}
				break;
			}
			case 0xE000:
			{
				uint8_t x = (opcode & 0x0F00) >> 8;
				if ((opcode & 0x00FF) == 0x009E)
				{
					if (isPressed(Chip8.V[x]))
						Chip8.PC += 2;
					break;
				}
				if (!(isPressed(Chip8.V[x])))
					Chip8.PC += 2;
				break;
			}
			case 0xF000:
			{
				uint8_t x = (opcode & 0x0F00) >> 8;
				switch (opcode & 0x00FF)
				{
				case 0x07:
					Chip8.V[x] = Chip8.DT;
					break;
				case 0x0A:
				{
					bool key_pressed = false;
					for (int i = 0; i < 16; i++)
					{
						if (Chip8.keypad[i] == 1)
						{
							Chip8.V[x] = i;
							key_pressed = true;
							Chip8.PC += 2;
							break;
						}
					}
					if (!key_pressed)
					{
						Chip8.PC -= 2;
					}
					break;
				}
				case 0x15:
					Chip8.DT = Chip8.V[x];
					break;
				case 0x18:
					Chip8.ST = Chip8.V[x];
					break;
				case 0x1E:
					Chip8.I += Chip8.V[x];
					break;
				case 0x29:
					uint8_t digit = Chip8.V[x];
					Chip8.I = 0x050 + (digit * 5);
					break;
				case 0x33:
					uint8_t value = Chip8.V[x];
					Chip8.memory[Chip8.I] = value / 100;
					Chip8.memory[Chip8.I + 1] = (value / 10) % 10;
					Chip8.memory[Chip8.I + 2] = value % 10;
					break;
				case 0x55:
				{
					for (int j = 0; j <= x; j++)
					{
						Chip8.memory[Chip8.I + j] = Chip8.V[j];
					}
					break;
				}
				case 0x65:
				{
					for (int j = 0; j <= x; j++)
					{
						Chip8.V[j] = Chip8.memory[Chip8.I + j];
					}
					break;
				}

				default:
					printf("Opcode não encontrado : %d \n", opcode);
					running = false;
					break;
				}
			}
			}
		}
		if (Chip8.DT > 0)
			Chip8.DT--;
		if (Chip8.ST > 0)
		{
			Chip8.ST--;
			SDL_PauseAudioDevice(audio_device, 0);
		}
		else
		{
			SDL_PauseAudioDevice(audio_device, 1);
		}
		clearScreen(renderer);
		drawScreen(renderer);
	}
	quitScreen(renderer, window, &audio_device);
	return 0;
}
