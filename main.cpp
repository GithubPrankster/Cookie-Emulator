#include <iostream>
#include <string>
#include <cstdlib>

#include <SDL.h>

#include "audioBoy.h"
#include "chip8.h"
#include "cookieEmu.h"


int main(int argc, char** argv) {
	std::cout << "Time to eat cookie!" << std::endl;

	if (argc != 3) {
		std::cout << "You gotta give me a file and size to work with!" << std::endl;
		std::cout << "Format : cookie const char* filename int size" << std::endl;
		return 1;
	}

	int size = atoi(argv[2]);

	CookieEmu mainCookie("Cookie Chip-8 Emulator", size);

	mainCookie.interpreter.load(argv[1]);
	mainCookie.run();
	return EXIT_SUCCESS;
}