
#include "SDL.h"
#include "SDL_image.h"
#include "SDL2_gfxPrimitives.h"
#include "SDL2_rotozoom.h"

#include "osint.h"
#include "e8910.h"
#include "vecx.h"

#define EMU_TIMER 20 /* the emulators heart beats at 20 milliseconds */

static SDL_Window *screen = NULL;
static SDL_Renderer *renderer= NULL;
static SDL_Texture *overlay = NULL;

static long scl_factor;
static long offx;
static long offy;

void osint_render(void){
	SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
	SDL_RenderClear(renderer);

	int v;
	for(v = 0; v < vector_draw_cnt; v++){
		Uint8 c = vectors_draw[v].color * 256 / VECTREX_COLORS;
		aalineRGBA(renderer,
				offx + vectors_draw[v].x0 / scl_factor,
				offy + vectors_draw[v].y0 / scl_factor,
				offx + vectors_draw[v].x1 / scl_factor,
				offy + vectors_draw[v].y1 / scl_factor,
				c, c, c, 0xff);
	}
	if(overlay){
		SDL_Rect dest_rect = {offx, offy, ((double)ALG_MAX_X / (double)scl_factor), ((double)ALG_MAX_Y / (double)scl_factor)};
		SDL_RenderCopy(renderer, overlay, NULL, &dest_rect);
	}
	SDL_RenderPresent(renderer);
}

static char *romfilename = "rom.dat";
static char *cartfilename = NULL;

static void init(){
	FILE *f;
	if(!(f = fopen(romfilename, "rb"))){
		perror(romfilename);
		exit(EXIT_FAILURE);
	}
	if(fread(rom, 1, sizeof (rom), f) != sizeof (rom)){
		printf("Invalid rom length\n");
		exit(EXIT_FAILURE);
	}
	fclose(f);

	memset(cart, 0, sizeof (cart));
	if(cartfilename){
		FILE *f;
		if(!(f = fopen(cartfilename, "rb"))){
			perror(cartfilename);
			exit(EXIT_FAILURE);
		}
		fread(cart, 1, sizeof (cart), f);
		fclose(f);
	}
}

void resize(int width, int height){
	long sclx, scly;

	long screenx = width;
	long screeny = height;

	sclx = ALG_MAX_X / width;
	scly = ALG_MAX_Y / height;

	scl_factor = sclx > scly ? sclx : scly;

	offx = (screenx - ALG_MAX_X / scl_factor) / 2;
	offy = (screeny - ALG_MAX_Y / scl_factor) / 2;
}

static void readevents(){
	SDL_Event e;
	while(SDL_PollEvent(&e)){
		switch(e.type){
			case SDL_QUIT:
				exit(EXIT_SUCCESS);
				break;
			case SDL_WINDOWEVENT:
				switch (e.window.event) {
					case SDL_WINDOWEVENT_RESIZED:
						resize(e.window.data1, e.window.data2);
						break;
					case SDL_WINDOWEVENT_SIZE_CHANGED:
						resize(e.window.data1, e.window.data2);
						break;
				}
				break;
			case SDL_KEYDOWN:
				switch(e.key.keysym.sym){
					case SDLK_ESCAPE:
						exit(EXIT_SUCCESS);
					case SDLK_a:
						snd_regs[14] &= ~0x01;
						break;
					case SDLK_s:
						snd_regs[14] &= ~0x02;
						break;
					case SDLK_d:
						snd_regs[14] &= ~0x04;
						break;
					case SDLK_f:
						snd_regs[14] &= ~0x08;
						break;
					case SDLK_LEFT:
						alg_jch0 = 0x00;
						break;
					case SDLK_RIGHT:
						alg_jch0 = 0xff;
						break;
					case SDLK_UP:
						alg_jch1 = 0xff;
						break;
					case SDLK_DOWN:
						alg_jch1 = 0x00;
						break;
					default:
						break;
				}
				break;
			case SDL_KEYUP:
				switch(e.key.keysym.sym){
					case SDLK_a:
						snd_regs[14] |= 0x01;
						break;
					case SDLK_s:
						snd_regs[14] |= 0x02;
						break;
					case SDLK_d:
						snd_regs[14] |= 0x04;
						break;
					case SDLK_f:
						snd_regs[14] |= 0x08;
						break;
					case SDLK_LEFT:
						alg_jch0 = 0x80;
						break;
					case SDLK_RIGHT:
						alg_jch0 = 0x80;
						break;
					case SDLK_UP:
						alg_jch1 = 0x80;
						break;
					case SDLK_DOWN:
						alg_jch1 = 0x80;
						break;
					default:
						break;
				}
				break;
			default:
				break;
		}
	}
}

void osint_emuloop(){
	Uint32 next_time = SDL_GetTicks() + EMU_TIMER;
	vecx_reset();
	for(;;){
		vecx_emu((VECTREX_MHZ / 1000) * EMU_TIMER);
		readevents();

		{
			Uint32 now = SDL_GetTicks();
			if(now < next_time)
				SDL_Delay(next_time - now);
			else
				next_time = now;
			next_time += EMU_TIMER;
		}
	}
}

void load_overlay(const char *filename){
	SDL_Surface *image;
	image = SDL_LoadBMP(filename);
	if(image){
		overlay = SDL_CreateTextureFromSurface(renderer, image);
		SDL_FreeSurface(image);
	}else{
		fprintf(stderr, "IMG_Load: %s\n", IMG_GetError());
	}
}

int main(int argc, char *argv[]){
	if(SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO) < 0){
		fprintf(stderr, "Failed to initialize SDL: %s\n", SDL_GetError());
		exit(-1);
	}
	SDL_CreateWindowAndRenderer(330*3/2, 410*3/2, SDL_WINDOW_RESIZABLE, &screen, &renderer);
	if(screen == NULL || renderer == NULL){
		fprintf(stderr, "Failed to initialize SDL window/renderer: %s\n", SDL_GetError());
		exit(-2);
	}

	resize(330*3/2, 410*3/2);

	if(argc > 1)
		cartfilename = argv[1];
	if(argc > 2)
		load_overlay(argv[2]);

	init();

	e8910_init_sound();
	osint_emuloop();
	e8910_done_sound();
	SDL_Quit();

	return 0;
}

