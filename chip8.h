#include <fstream>
#include <vector>
#include <random>

struct Chip8 {
	uint16_t operation;

	uint8_t mem[4096];
	uint8_t registers[16];
	uint8_t superRegisters[8];

	uint16_t indexRegister, prgCounter;

	std::vector<uint8_t> screen;
	uint8_t delayTime, soundTime;

	uint16_t stack[16];
	uint16_t stackPointer;

	uint8_t keys[16];

	std::mt19937 rnd;
	bool drawn;

	audioBoy audioThing;

	bool modeSuper;

	void init(bool mode = false) {

		prgCounter = 512;
		operation = 0;
		indexRegister = 0;
		stackPointer = 0;

		for (int i = 0; i < 16; i++) {
			registers[i] = 0;
			stack[i] = 0;
			keys[i] = 0;
		}

		screen.resize(64 * 32);

		for (int r = 0; r < 4096; r++) {
			mem[r] = 0;
		}

		constexpr uint8_t chip8_fontset[80] =
		{
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

		constexpr uint8_t superChip8_fontset[160] = {
			0xff, 0xff, 0xc3, 0xc3, 0xc3, 0xc3, 0xc3, 0xc3, 0xff, 0xff, // 0
			0x18, 0x78, 0x78, 0x18, 0x18, 0x18, 0x18, 0x18, 0xff, 0xff, // 1
			0xff, 0xff, 0x03, 0x03, 0xff, 0xff, 0xc0, 0xc0, 0xff, 0xff, // 2
			0xff, 0xff, 0x03, 0x03, 0xff, 0xff, 0x03, 0x03, 0xff, 0xff, // 3
			0xc3, 0xc3, 0xc3, 0xc3, 0xff, 0xff, 0x03, 0x03, 0x03, 0x03, // 4
			0xff, 0xff, 0xc0, 0xc0, 0xff, 0xff, 0x03, 0x03, 0xff, 0xff, // 5
			0xff, 0xff, 0xc0, 0xc0, 0xff, 0xff, 0xc3, 0xc3, 0xff, 0xff, // 6
			0xff, 0xff, 0x03, 0x03, 0x06, 0x0c, 0x18, 0x18, 0x18, 0x18, // 7
			0xff, 0xff, 0xc3, 0xc3, 0xff, 0xff, 0xc3, 0xc3, 0xff, 0xff, // 8
			0xff, 0xff, 0xc3, 0xc3, 0xff, 0xff, 0x03, 0x03, 0xff, 0xff, // 9
			0x7e, 0xff, 0xc3, 0xc3, 0xc3, 0xff, 0xff, 0xc3, 0xc3, 0xc3, // A
			0xfc, 0xfc, 0xc3, 0xc3, 0xfc, 0xfc, 0xc3, 0xc3, 0xfc, 0xfc, // B
			0x3c, 0xff, 0xc3, 0xc0, 0xc0, 0xc0, 0xc0, 0xc3, 0xff, 0x3c, // C
			0xfc, 0xfe, 0xc3, 0xc3, 0xc3, 0xc3, 0xc3, 0xc3, 0xfe, 0xfc, // D
			0xff, 0xff, 0xc0, 0xc0, 0xff, 0xff, 0xc0, 0xc0, 0xff, 0xff, // E
			0xff, 0xff, 0xc0, 0xc0, 0xff, 0xff, 0xc0, 0xc0, 0xc0, 0xc0  // F
		};


		for (int q = 0; q < 80; q++) {
			mem[q] = chip8_fontset[q];
		}

		for (int d = 0; d < 160; d++) {
			mem[d + 80] = superChip8_fontset[d];
		}

		delayTime = 0;
		soundTime = 0;
		drawn = false;

		rnd = std::mt19937(std::random_device{}());
	}

	//Load into our memory some file!
	void load(std::string filename, unsigned pos = 0x200) {
		for (std::ifstream f(filename, std::ios::binary); f.good(); )
			mem[pos++ & 0xFFF] = f.get();
	}

	void progressPrg(bool skip = false) {
		prgCounter = (prgCounter + (skip ? 4 : 2)) & 0xFFF;
	}


	void opcodeUncommon(bool &killSwitch) {
		std::cout << "What the hell was the opcode: " << std::hex << operation << " all about?" << std::endl;
		std::cout << "Case 0x0000." << std::endl;
		killSwitch = false;
	}

	void opcode0x00E0() {
		std::fill(screen.begin(), screen.end(), 0);
		drawn = true;
		progressPrg();
	}

	void opcode0x00EE() {
		--stackPointer;
		prgCounter = stack[stackPointer];
		progressPrg();
	}

	void opcode0x0000(bool &killSwitch) {
		//Starting off with some technical operations:
		switch (operation & 0x00FF) {

			//Screen clear, sets "drawn" true so ya know
			//some drawing 'biss happened.
		case 0x00E0:
			opcode0x00E0();
			break;

			//Subroutine return! You know, that thing from asm.
		case 0x00EE:
			opcode0x00EE();
			break;

			//In case something really weird gets called around here.
		default:
			opcodeUncommon(killSwitch);
			break;
		}
	}

	void opcode0x1000() {
		prgCounter = operation & 0x0FFF;
	}

	void opcode0x2000() {
		stack[stackPointer] = prgCounter;
		++stackPointer;
		prgCounter = operation & 0x0FFF;
	}

	void opcode0x3000() {
		if (registers[(operation & 0x0F00) >> 8] == (operation & 0x00FF))
			progressPrg(true);
		else
			progressPrg();
	}

	void opcode0x4000() {
		if (registers[(operation & 0x0F00) >> 8] != (operation & 0x00FF))
			progressPrg(true);
		else
			progressPrg();
	}

	void opcode0x5000() {
		if (registers[(operation & 0x0F00) >> 8] == registers[(operation & 0x00F0) >> 4])
			progressPrg(true);
		else
			progressPrg();
	}

	void opcode0x6000() {
		registers[(operation & 0x0F00) >> 8] = operation & 0x00FF;
		progressPrg();
	}

	void opcode0x7000() {
		registers[(operation & 0x0F00) >> 8] += operation & 0x00FF;
		progressPrg();
	}


	void opcode0x8XY0() {
		registers[(operation & 0x0F00) >> 8] = registers[(operation & 0x00F0) >> 4];
		progressPrg();
	}

	void opcode0x8XY1() {
		registers[(operation & 0x0F00) >> 8] |= registers[(operation & 0x00F0) >> 4];
		progressPrg();
	}

	void opcode0x8XY2() {
		registers[(operation & 0x0F00) >> 8] &= registers[(operation & 0x00F0) >> 4];
		progressPrg();
	}

	void opcode0x8XY3() {
		registers[(operation & 0x0F00) >> 8] &= registers[(operation & 0x00F0) >> 4];
		progressPrg();
	}

	void opcode0x8XY4() {
		registers[(operation & 0x0F00) >> 8] += registers[(operation & 0x00F0) >> 4];
		if (registers[(operation & 0x00F0) >> 4] > (0xFF - registers[(operation & 0x0F00) >> 8]))
			registers[0xF] = 1;
		else
			registers[0xF] = 0;
		progressPrg();
	}

	void opcode0x8XY5() {
		if (registers[(operation & 0x00F0) >> 4] > registers[(operation & 0x0F00) >> 8])
			registers[0xF] = 0;
		else
			registers[0xF] = 1;
		registers[(operation & 0x0F00) >> 8] -= registers[(operation & 0x00F0) >> 4];
		progressPrg();
	}

	void opcode0x8XY6() {
		registers[0xF] = registers[(operation & 0x0F00) >> 8] & 0x1;
		registers[(operation & 0x0F00) >> 8] >>= 1;
		progressPrg();
	}

	void opcode0x8XY7() {
		if (registers[(operation & 0x0F00) >> 8] > registers[(operation & 0x00F0) >> 4])
			registers[0xF] = 0;
		else
			registers[0xF] = 1;
		registers[(operation & 0x0F00) >> 8] = registers[(operation & 0x00F0) >> 4] - registers[(operation & 0x0F00) >> 8];
		progressPrg();
	}

	void opcode0x8XYE() {
		registers[0xF] = registers[(operation & 0x0F00) >> 8] >> 7;
		registers[(operation & 0x0F00) >> 8] <<= 1;
		progressPrg();
	}

	void opcode0x8000(bool &killSwitch) {
		switch (operation & 0x000F) {
			//Set a register to the value of another one. 
		case 0x0000:
			opcode0x8XY0();
			break;
			//Set a register to a match of OR with another one!
		case 0x0001:
			opcode0x8XY1();
			break;
			//Set a register to a match of AND with another one!
		case 0x0002:
			opcode0x8XY2();
			break;
			//Set a register to a match of XOR with another one!
			//Maybe we should turn this into a big competition.
		case 0x0003:
			opcode0x8XY3();
			break;

			//Add some register to another one. However,
			//Set the special F flag to 1 if a carry happened.
		case 0x0004:
			opcode0x8XY4();
			break;

			//If a borrow had happened, set the special F flag down,
			//unless it didn't. Oh, also subtract another register from some register.
		case 0x0005:
			opcode0x8XY5();
			break;

			//Set the special F flag to the least cool value of a register,
			//then shift 'em to the right once.
		case 0x0006:
			opcode0x8XY6();
			break;

			//If a borrow had happened, set the special F flag down (or not)
			//then do the inverse subtraction order of 0x0005.
		case 0x0007:
			opcode0x8XY7();
			break;

			//Set the special F flag to the coolest value of a register,
			//then shift 'em to the left once.
		case 0x000E:
			opcode0x8XYE();
			break;

			//Hey, gotta always make sure.
		default:
			opcodeUncommon(killSwitch);
			break;
		}
	}

	void opcode0x9000() {
		if (registers[(operation & 0x0F00) >> 8] != registers[(operation & 0x00F0) >> 4])
			progressPrg(true);
		else
			progressPrg();
	}

	void opcode0xA000() {
		indexRegister = operation & 0x0FFF;
		progressPrg();
	}

	void opcode0xB000() {
		prgCounter = (operation & 0x0FFF) + registers[0];
	}

	void opcode0xC000() {
		registers[(operation & 0x0F00) >> 8] = std::uniform_int_distribution<>(0, 255)(rnd) & (operation & 0x00FF);
		progressPrg();
	}

	void opcode0xD000() {
		uint16_t x = registers[(operation & 0x0F00) >> 8];
		uint16_t y = registers[(operation & 0x00F0) >> 4];
		uint16_t height = operation & 0x000F;
		uint16_t pixel;

		registers[0xF] = 0;
		for (int yline = 0; yline < height; yline++)
		{
			pixel = mem[indexRegister + yline];
			for (int xline = 0; xline < 8; xline++)
			{
				if ((pixel & (0x80 >> xline)) != 0)
				{
					if (screen[(x + xline + ((y + yline) * 64))] == 1)
					{
						registers[0xF] = 1;
					}
					screen[x + xline + ((y + yline) * 64)] ^= 1;
				}
			}
		}

		drawn = true;
		progressPrg();
	}

	void opcode0xE09E() {
		if (keys[registers[(operation & 0x0F00) >> 8]] != 0)
			progressPrg(true);
		else
			progressPrg();
	}

	void opcode0xE0A1() {
		if (keys[registers[(operation & 0x0F00) >> 8]] == 0)
			progressPrg(true);
		else
			progressPrg();
	}

	void opcode0xE000(bool &killSwitch) {
		switch (operation & 0x00FF) {

			//Skip if some key went unpressed.
		case 0x009E:
			opcode0xE09E();
			break;

			//Or skip if it actually got pressed.
		case 0x00A1:
			opcode0xE0A1();
			break;

			//Remember kids, never put in ilegal opcodes in your programs! Don't copy floppies too.
		default:
			opcodeUncommon(killSwitch);
			break;
		}
	}

	void opcode0xF007() {
		registers[(operation & 0x0F00) >> 8] = delayTime;
		progressPrg();
	}

	void opcode0xF00A() {
		bool keyPress = false;

		for (int i = 0; i < 16; ++i)
		{
			if (keys[i] != 0)
			{
				registers[(operation & 0x0F00) >> 8] = i;
				keyPress = true;
			}
		}

		//Come on! Just press the key! :(
		if (!keyPress)
			return;

		progressPrg();
	}

	void opcode0xF015() {
		delayTime = registers[(operation & 0x0F00) >> 8];
		progressPrg();
	}

	void opcode0xF018() {
		soundTime = registers[(operation & 0x0F00) >> 8];
		progressPrg();
	}

	void opcode0xF01E() {
		if (indexRegister + registers[(operation & 0x0F00) >> 8] > 0xFFF)
			registers[0xF] = 1;
		else
			registers[0xF] = 0;
		indexRegister += registers[(operation & 0x0F00) >> 8];
		progressPrg();
	}

	void opcode0xF029() {
		indexRegister = registers[(operation & 0x0F00) >> 8] * 0x5;
		progressPrg();
	}

	void opcode0xF033() {
		mem[indexRegister] = registers[(operation & 0x0F00) >> 8] / 100;
		mem[indexRegister + 1] = (registers[(operation & 0x0F00) >> 8] / 10) % 10;
		mem[indexRegister + 2] = registers[(operation & 0x0F00) >> 8] % 10;
		progressPrg();
	}

	void opcode0xF055() {
		for (int i = 0; i <= ((operation & 0x0F00) >> 8); ++i)
			mem[indexRegister + i] = registers[i];

		indexRegister += ((operation & 0x0F00) >> 8) + 1;
		progressPrg();
	}

	void opcode0xF065() {
		for (int i = 0; i <= ((operation & 0x0F00) >> 8); ++i)
			registers[i] = mem[indexRegister + i];

		indexRegister += ((operation & 0x0F00) >> 8) + 1;
		progressPrg();
	}

	void opcode0xF000(bool &killSwitch) {
		switch (operation & 0x00FF)
		{

			//Set a register to the delay counter.
		case 0x0007:
			opcode0xF007();
			break;

			//Wait for a key press and store that badboy in a register!
		case 0x000A:
			opcode0xF00A();
			break;

		//Set the delay count to whatever's inside that register.
		case 0x0015:
			opcode0xF015();
			break;

			//Set the sound count to whatever's inside that register.
		case 0x0018:
			opcode0xF018();
			break;

			//Set the special flag up if the (not really) famous range overflow happened.
			//Oh, also increase the index register with a value in a register.
		case 0x001E:
			opcode0xF01E();
			break;

			//Set the index register to a sprite in some register. 
		case 0x0029:
			opcode0xF029();
			break;

			//It's time for a Binary Coded Decimal! Of a register, specifically.
		case 0x0033:
			opcode0xF033();
			break;

			//Store some registers in memory starting from where in the world
			//that index register was. Also increase it by the amount it got.
		case 0x0055:
			opcode0xF055();
			break;

			//Do the first thing from 0x0055 but inverse! 
		case 0x0065:
			opcode0xF065();
			break;

			//Just to be sure!
		default:
			opcodeUncommon(killSwitch);
			break;
		}
	}

	void runOperation(bool& alive) {
		operation = mem[prgCounter] << 8 | mem[prgCounter + 1];

		//Ok opcodes, present to me what you have obtained!
		switch (operation & 0xF000) {
		case 0x0000:
			opcode0x0000(alive);
			break;

			//Jump to that nice address the mask provided us with.
		case 0x1000:
			opcode0x1000();
			break;

			//Call a subroutine in some address! Which one?
			//The program knows! Don't ask me.
		case 0x2000:
			opcode0x2000();
			break;

			//Skip if a register really does contain some weird number between 0-255.
			//Again, ask the program about it.
		case 0x3000:
			opcode0x3000();
			break;

			//Now skip if it didn't contain the number after all. Sorry!
		case 0x4000:
			opcode0x4000();
			break;

			//Skip if a register contains the same stuff as another one. 
		case 0x5000:
			opcode0x5000();
			break;

			//Set a register to some number. Cool!
		case 0x6000:
			opcode0x6000();
			break;

			//Why not instead just add that number into the register?
		case 0x7000:
			opcode0x7000();
			break;

			//It's the Eight Fighters! Oh no!
		case 0x8000:
			opcode0x8000(alive);
			break;

			//Skip if some register is different from another one.
		case 0x9000:
			opcode0x9000();
			break;

			//Sets index register to a cool address
			//the rest of the hexcode contains!
		case 0xA000:
			opcode0xA000();
			break;

			//Sets the program counter to a cool address
			//Plus the first register's stuff.
		case 0xB000:
			opcode0xB000();
			break;

			//Sets a cool register to a random 0-255 number
			//with a mask to only get that stuff.
		case 0xC000:
			opcode0xC000();
			break;

			//The draw command! You can set a height for the sprite along with the position of it.
		case 0xD000:
			opcode0xD000();
			break;

		//You all know about E? I do. He has some clicky friends!
		case 0xE000:
			opcode0xE000(alive);
			break;

			//F's friends are on time!
		case 0xF000:
			opcode0xF000(alive);
			break;

			//No Jim, you cannot add an opcode called "KEY where, get".
		default:
			opcodeUncommon(alive);
			break;
		}
	}

	void runCycle(bool& alive) {
		runOperation(alive);

		if (delayTime > 0) {
			--delayTime;
		}

		if (soundTime > 0) {
			--soundTime;
			if (soundTime == 1) {
				audioThing.play();
				--soundTime;
			}
		}
	}
};
