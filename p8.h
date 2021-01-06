#ifndef CELESTE_RAYLIB_P8_H
#define CELESTE_RAYLIB_P8_H

#include <stdint.h>

#define P8_SCR_SIZE 128

#define P8_BTN_L  (1u << 0)
#define P8_BTN_R  (1u << 1)
#define P8_BTN_U  (1u << 2)
#define P8_BTN_D  (1u << 3)
#define P8_BTN_O  (1u << 4)
#define P8_BTN_X  (1u << 5)

void p8_init();
void p8_update(unsigned buttons);
void *p8_draw();
void p8_audio(unsigned samples, int16_t *pcm);

#endif
