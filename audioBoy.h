#include <SDL_audio.h>

struct audioBoy {
	SDL_AudioSpec specs{};
	SDL_AudioDeviceID dev;

	float osc_freq = 440.0f;
	float osc_phase = 0.0f;
	float gate_phase = 0.0f;
	float gate_length = 0.2f;

	void init() {
		specs.freq = 96000;
		specs.format = AUDIO_F32SYS;
		specs.channels = 1;
		specs.samples = 4096;
		specs.userdata = this;
		specs.callback = [](void* audio, Uint8 * stream, int len) {
			((audioBoy*)audio)->callback((float*)stream, len / sizeof(float));
		};
		dev = SDL_OpenAudioDevice(nullptr, 0, &specs, &specs, SDL_AUDIO_ALLOW_FREQUENCY_CHANGE);
		if (dev != NULL) {
			SDL_PauseAudioDevice(dev, 0);
		}
		else {
			std::cout << "Sorry, no audio device for you. " << SDL_GetError() << std::endl;
		}
		
	}

	void play() {
		gate_phase = 0.25f;
	}

	void callback(float* targ, int samples) {
		for (int p = 0; p < samples; ++p) {
			targ[p] = sample();
		}
	}

	float sample() {
		const float osc = 1.0 - float(osc_phase > 0.5) * 2.0; 
		osc_phase = std::fmod(osc_phase + osc_freq / specs.freq, 1.0f); 

		if (gate_phase > 0.0f) {
			gate_phase -= gate_length / specs.freq;
			return osc * gate_phase; 
		}
		else {
			return 0.0f; 
		}
	}
};
