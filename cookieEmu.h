#include <chrono>
#include <thread>

constexpr uint8_t sdlKeys[16] = {
	SDLK_x,
	SDLK_1,
	SDLK_2,
	SDLK_3,
	SDLK_q,
	SDLK_w,
	SDLK_e,
	SDLK_a,
	SDLK_s,
	SDLK_d,
	SDLK_z,
	SDLK_c,
	SDLK_4,
	SDLK_r,
	SDLK_f,
	SDLK_v,
};


struct CookieEmu {
	SDL_Window* window;
	SDL_Renderer* renderer;

	Chip8 interpreter;

	int scrWidth;
	int scrHeight;

	CookieEmu(const char* windowName, int widthMultiplier) {
		scrWidth = 64 * widthMultiplier;
		scrHeight = 32 * widthMultiplier;

		if (SDL_Init(SDL_INIT_EVERYTHING) < 0) {
			std::cout << "nothing was initialized lul" << SDL_GetError() << std::endl;
		}

		window = SDL_CreateWindow(windowName, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, scrWidth, scrHeight, SDL_WINDOW_SHOWN);
		if (window == NULL) {
			std::cout << "window could not initialize:" << SDL_GetError() << std::endl;
		}

		renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
		SDL_RenderSetLogicalSize(renderer, scrWidth, scrHeight);
		if (renderer == NULL) {
			std::cout << "renderer could not initialize:" << SDL_GetError() << std::endl;
		}

		interpreter.init();
		interpreter.audioThing.init();
	}
	~CookieEmu() {
		SDL_DestroyRenderer(renderer);
		SDL_DestroyWindow(window);
		SDL_Quit();
	}

	void eventLoop(bool& runArg) {
		SDL_Event event;

		while (SDL_PollEvent(&event) != 0) {
			if (event.type == SDL_QUIT) {
				runArg = false;
			}

			else if (event.type == SDL_KEYDOWN) {
				switch (event.key.keysym.sym) {

				case SDLK_ESCAPE:
					runArg = false;
					break;

				default:
					break;
				}
				for (int i = 0; i < 16; ++i) {
					if (event.key.keysym.sym == sdlKeys[i]) {
						interpreter.keys[i] = 1;
					}
				}
			}
			if (event.type == SDL_KEYUP) {
				for (int i = 0; i < 16; ++i) {
					if (event.key.keysym.sym == sdlKeys[i]) {
						interpreter.keys[i] = 0;
					}
				}
			}
		}
	}

	void run() {
		SDL_Texture* sdlTexture = SDL_CreateTexture(renderer,
			SDL_PIXELFORMAT_ARGB8888,
			SDL_TEXTUREACCESS_STREAMING,
			64, 32);

		uint32_t pixels[64 * 32];
		bool running = true;

		while (running) {
			interpreter.runCycle(running);
			eventLoop(running);

			if (interpreter.drawn) {
				interpreter.drawn = false;

				for (int i = 0; i < 64 * 32; ++i) {
					uint8_t pixel = interpreter.screen[i];
					pixels[i] = (0x00FFFFFF * pixel) | 0xFF000000;
				}

				SDL_UpdateTexture(sdlTexture, NULL, pixels, 64 * sizeof(Uint32));

				SDL_RenderClear(renderer);
				SDL_RenderCopy(renderer, sdlTexture, NULL, NULL);
				SDL_RenderPresent(renderer);
			}

			std::this_thread::sleep_for(std::chrono::microseconds(1200));
		}
	}
};
