#include "audio.h"

#include <SDL.h>
#include <math.h>
#include <stdio.h>

#define SAMPLE_RATE 48000
#define CHANNELS 2
#define PI_F 3.14159265358979323846f

static SDL_AudioDeviceID audio_device;
static double music_time;
static int jump_samples_left;
static double jump_phase;
static SDL_RWops *bgm_file;
static Sint64 bgm_data_offset;
static Uint32 bgm_data_length;
static Uint32 bgm_bytes_read;
static int bgm_sample_rate;
static int bgm_channels;
static double bgm_resample_phase;
static float bgm_left;
static float bgm_right;

static const float melody[] = {
    261.63f, 329.63f, 392.00f, 329.63f,
    293.66f, 349.23f, 440.00f, 349.23f,
    246.94f, 293.66f, 392.00f, 293.66f,
    220.00f, 329.63f, 392.00f, 493.88f
};

static float triangle_wave(double phase) {
    return (float)(2.0 * fabs(2.0 * (phase - floor(phase + 0.5))) - 1.0);
}

static bool read_bgm_frame(void) {
    Sint16 samples[2] = {0, 0};
    Uint32 frame_size = (Uint32)(bgm_channels * sizeof(Sint16));

    if (bgm_file == NULL || frame_size == 0) return false;

    if (bgm_bytes_read + frame_size > bgm_data_length) {
        SDL_RWseek(bgm_file, bgm_data_offset, RW_SEEK_SET);
        bgm_bytes_read = 0;
    }

    if (SDL_RWread(bgm_file, samples, frame_size, 1) != 1) return false;
    bgm_bytes_read += frame_size;

    bgm_left = samples[0] / 32768.0f;
    bgm_right = samples[bgm_channels > 1 ? 1 : 0] / 32768.0f;
    return true;
}

static void audio_callback(void *userdata, Uint8 *stream, int len) {
    (void)userdata;

    float *output = (float *)stream;
    int frames = len / (int)(sizeof(float) * CHANNELS);
    const double step_duration = 0.25;
    const int melody_length = (int)(sizeof(melody) / sizeof(melody[0]));

    for (int i = 0; i < frames; i++) {
        float left;
        float right;

        if (bgm_file != NULL) {
            left = bgm_left * 0.7f;
            right = bgm_right * 0.7f;
            bgm_resample_phase += bgm_sample_rate;
            while (bgm_resample_phase >= SAMPLE_RATE) {
                read_bgm_frame();
                bgm_resample_phase -= SAMPLE_RATE;
            }
        } else {
            int note_index = (int)(music_time / step_duration) % melody_length;
            double note_time = fmod(music_time, step_duration);
            double phase = fmod(music_time * melody[note_index], 1.0);
            float note_envelope =
                note_time < 0.21 ? 1.0f : (float)((step_duration - note_time) / 0.04);
            float sample = triangle_wave(phase) * 0.09f * note_envelope;

            int bass_index = (note_index / 4) * 4;
            double bass_phase =
                fmod(music_time * (melody[bass_index] * 0.5f), 1.0);
            sample += sinf((float)(bass_phase * 2.0 * PI_F)) * 0.05f;
            left = sample;
            right = sample;
        }

        if (jump_samples_left > 0) {
            float progress =
                1.0f - (float)jump_samples_left / (SAMPLE_RATE * 0.18f);
            float frequency = 420.0f + 780.0f * progress;
            float envelope =
                (float)jump_samples_left / (SAMPLE_RATE * 0.18f);
            jump_phase = fmod(jump_phase + frequency / SAMPLE_RATE, 1.0);
            float jump_sample =
                sinf((float)(jump_phase * 2.0 * PI_F)) * 0.24f * envelope;
            left += jump_sample;
            right += jump_sample;
            jump_samples_left--;
        }

        output[i * CHANNELS] = left;
        output[i * CHANNELS + 1] = right;
        music_time += 1.0 / SAMPLE_RATE;
    }
}

static bool load_bgm(const char *path) {
    char chunk_id[4];
    Uint32 chunk_size;
    Uint16 audio_format = 0;
    Uint16 bits_per_sample = 0;
    bool found_format = false;

    bgm_file = SDL_RWFromFile(path, "rb");
    if (bgm_file == NULL) {
        fprintf(stderr, "BGM not loaded (%s); using generated music.\n", path);
        return false;
    }

    if (SDL_RWread(bgm_file, chunk_id, 4, 1) != 1 ||
        SDL_memcmp(chunk_id, "RIFF", 4) != 0) {
        goto invalid_wav;
    }
    SDL_ReadLE32(bgm_file);
    if (SDL_RWread(bgm_file, chunk_id, 4, 1) != 1 ||
        SDL_memcmp(chunk_id, "WAVE", 4) != 0) {
        goto invalid_wav;
    }

    while (SDL_RWread(bgm_file, chunk_id, 4, 1) == 1) {
        chunk_size = SDL_ReadLE32(bgm_file);

        if (SDL_memcmp(chunk_id, "fmt ", 4) == 0 && chunk_size >= 16) {
            audio_format = SDL_ReadLE16(bgm_file);
            bgm_channels = SDL_ReadLE16(bgm_file);
            bgm_sample_rate = (int)SDL_ReadLE32(bgm_file);
            SDL_RWseek(bgm_file, 6, RW_SEEK_CUR);
            bits_per_sample = SDL_ReadLE16(bgm_file);
            SDL_RWseek(bgm_file, chunk_size - 16, RW_SEEK_CUR);
            found_format = true;
        } else if (SDL_memcmp(chunk_id, "data", 4) == 0 && found_format) {
            bgm_data_offset = SDL_RWtell(bgm_file);
            bgm_data_length = chunk_size;
            break;
        } else {
            SDL_RWseek(bgm_file, chunk_size, RW_SEEK_CUR);
        }

        if (chunk_size & 1) SDL_RWseek(bgm_file, 1, RW_SEEK_CUR);
    }

    if (audio_format != 1 || bits_per_sample != 16 ||
        (bgm_channels != 1 && bgm_channels != 2) ||
        bgm_data_length == 0) {
        goto invalid_wav;
    }

    bgm_bytes_read = 0;
    bgm_resample_phase = 0.0;
    read_bgm_frame();
    return true;

invalid_wav:
    fprintf(stderr, "BGM must be a PCM 16-bit mono/stereo WAV file.\n");
    SDL_RWclose(bgm_file);
    bgm_file = NULL;
    return false;
}

bool audio_init(void) {
    SDL_AudioSpec desired = {0};
    SDL_AudioSpec obtained = {0};

    desired.freq = SAMPLE_RATE;
    desired.format = AUDIO_F32SYS;
    desired.channels = CHANNELS;
    desired.samples = 1024;
    desired.callback = audio_callback;

    audio_device = SDL_OpenAudioDevice(NULL, 0, &desired, &obtained, 0);
    if (audio_device == 0) {
        fprintf(stderr, "Audio initialization failed: %s\n", SDL_GetError());
        return false;
    }

    music_time = 0.0;
    jump_samples_left = 0;
    jump_phase = 0.0;
    bgm_file = NULL;
    bgm_data_length = 0;
    bgm_bytes_read = 0;
    load_bgm("assets/audio/bgm.wav");
    SDL_PauseAudioDevice(audio_device, 0);
    return true;
}

void audio_shutdown(void) {
    if (audio_device != 0) {
        SDL_CloseAudioDevice(audio_device);
        audio_device = 0;
    }
    if (bgm_file != NULL) {
        SDL_RWclose(bgm_file);
        bgm_file = NULL;
    }
}

void audio_play_jump(void) {
    if (audio_device == 0) return;

    SDL_LockAudioDevice(audio_device);
    jump_samples_left = (int)(SAMPLE_RATE * 0.18f);
    jump_phase = 0.0;
    SDL_UnlockAudioDevice(audio_device);
}
