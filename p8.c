#include "p8.h"

#include <stddef.h>
#include "ccleste/celeste.h"

#include <math.h>
#include <stdarg.h>
#include <stdio.h>
#include <string.h>

static uint8_t fb[P8_SCR_SIZE][P8_SCR_SIZE][4];
static unsigned cur_buttons;

void *p8_draw()
{
  Celeste_P8_draw();
  return (void *)&fb[0][0][0];
}

void p8_update(unsigned buttons)
{
  cur_buttons = buttons;
  Celeste_P8_update();
}

// Interface to ccleste

#include "ccleste/tilemap.h"
#include "res/p8_font.h"
#include "res/p8_gfx.h"
#include "res/p8_sfx_defs.h"
#include "res/p8_sfx.h"

// Graphics

static int camera_x, camera_y;

static const uint8_t pal_default[16][3] = {
  {0x00, 0x00, 0x00},
  {0x1d, 0x2b, 0x53},
  {0x7e, 0x25, 0x53},
  {0x00, 0x87, 0x51},
  {0xab, 0x52, 0x36},
  {0x5f, 0x57, 0x4f},
  {0xc2, 0xc3, 0xc7},
  {0xff, 0xf1, 0xe8},
  {0xff, 0x00, 0x4d},
  {0xff, 0xa3, 0x00},
  {0xff, 0xec, 0x27},
  {0x00, 0xe4, 0x36},
  {0x29, 0xad, 0xff},
  {0x83, 0x76, 0x9c},
  {0xff, 0x77, 0xa8},
  {0xff, 0xcc, 0xaa}
};
static uint8_t pal[16][3];

static inline void pix(int x, int y, int col)
{
  if (x >= 0 && x < 128 && y >= 0 && y < 128) {
    fb[y][x][0] = pal[col][0];
    fb[y][x][1] = pal[col][1];
    fb[y][x][2] = pal[col][2];
    fb[y][x][3] = 0xff;
  }
}

static inline void spr(int cx, int cy, int sx, int sy)
{
  for (int y = 0; y < 8; y++) if (sy + y >= 0 && sy + y < 128)
    for (int x = 0; x < 8; x++) {
      uint8_t col = (p8_gfx[cy][cx][y] >> (4 * x)) & 0xf;
      if (col != 0) pix(sx + x, sy + y, col);
    }
}

static inline void spr_flippable(int cx, int cy, int sx, int sy, int fx, int fy)
{
  for (int y = 0; y < 8; y++) if (sy + y >= 0 && sy + y < 128)
    for (int x = 0; x < 8; x++) {
      uint8_t col = (p8_gfx[cy][cx][fy ? (7 - y) : y] >> (4 * (fx ? (7 - x) : x))) & 0xf;
      if (col != 0) pix(sx + x, sy + y, col);
    }
}

static inline void fillrect(int x0, int y0, int x1, int y1, int col)
{
  for (int y = y0; y <= y1; y++)
    for (int x = x0; x <= x1; x++)
      pix(x, y, col);
}

// Audio

static struct {
  unsigned snd_id;
  unsigned note;
  unsigned samples;
  float phase;
} channels[4];

static unsigned music_mask = 0;
static int last_sfx_ch = 3;

// A3 (33) = 440 Hz
static inline float freq(unsigned pitch)
{
  return 440.0f * powf(2.0f, (float)((int)pitch - 33) / 12.0f);
}

typedef float (*osc_t)(float);

// PRNG
static uint32_t seed = 0x20210106;
static inline float my_rand()
{
  uint32_t i = 0;
  seed = seed * 1103515245 + 12345;
  i = (seed & 0x7fff0000);
  seed = seed * 1103515245 + 12345;
  i = (i >> 1) | ((seed >> 16) & 0x7fff);
  return ((float)i / 0x3fffffff * 2) - 1;
}

// Implementations by picolove
// Actual PICO-8 implementations are more detailed
#define frac(x) ((x) - (int)(x))
static float osc_tri(float x) { return (2 * fabsf(2 * frac(x + 0.25f) - 1) - 1) * 0.7f; }
static float osc_tsaw(float x) {
  x = frac(x);
  return (((x < 0.875f) ? (x * 16 / 7) : ((x - 1) * 16)) - 1) * 0.7f;
}
static float osc_saw(float x) { return (frac(x) - 0.5f) * 0.9f; }
static float osc_sqr(float x) { return (frac(x) < 0.5000f) ? 1.0f/3 : -1.0f/3; }
static float osc_pulse(float x) { return (frac(x) < 0.3125f) ? 1.0f/3 : -1.0f/3; }
static float osc_organ(float x) {
  float y = fmodf(x * 4, 2);
  float z = fmodf(x * 2, 2);
  return (fabsf(y-1)-0.5 + (fabsf(z-1)-0.5)/2-0.1) * 0.7;
}
static float osc_noise(float x) {
  const float tscale = 0.11288053831187f;
  static float lastx = 0;
  static float smp0 = 0, smp1 = 0;
  float scale = (x - lastx) / tscale;
  smp0 = smp1;
  smp1 = (smp0 + scale * my_rand()) / (1 + scale);
  lastx = x;
  return fminf(fmaxf((smp0+smp1)*4/3*(1.75f-scale), -1), 1) * 0.7f;
}
static float osc_phaser(float x) {
  float y = fmodf(x * 2, 2);
  float z = fmodf(x * (127.0f/64), 2);
  return (fabsf(y-1)-0.5f + (fabsf(z-1)-0.5f)/2) - 1.0f/4;
}

static const osc_t osc[8] = {
  osc_tri, osc_tsaw, osc_saw, osc_sqr,
  osc_pulse, osc_organ, osc_noise, osc_phaser,
};

// 22050 Hz 16-bit mono
void p8_audio(unsigned block_samples, int16_t *pcm)
{
  for (unsigned i = 0; i < block_samples; i++) pcm[i] = 0;
  for (int ch = 0; ch < 4; ch++) {
    unsigned snd_id = channels[ch].snd_id;
    if (snd_id == 0xff) continue;
    unsigned note = channels[ch].note;
    unsigned samples = channels[ch].samples;
    float phase = channels[ch].phase;
    const p8_snd *snd = &p8_sfx[snd_id];
    for (unsigned i = 0; i < block_samples; i++) {
      float f = freq(snd->notes[note].pitch);
      if (snd->notes[note].vol != 0) {
        // Oscillator
        float x = phase + (float)samples / 22050 * f;
        float value = osc[snd->notes[note].wform](x);
        value *= (float)snd->notes[note].vol / 7;
        pcm[i] += (int16_t)roundf(value * 8191.5f);
      }

      // Update
      samples++;
      if (samples == 183 * snd->spd) {
        phase += (float)samples / 22050 * f;
        samples = 0;
        note++;
        if (snd->lpstart < snd->lpend && note >= snd->lpend) {
          note = snd->lpstart;
        } else if (note >= 32) {
          snd_id = 0xff;
          break;
        }
      }
    }
    channels[ch].snd_id = snd_id;
    channels[ch].note = note;
    channels[ch].samples = samples;
    channels[ch].phase = phase;
  }
}

// Interface

static int p8_call(CELESTE_P8_CALLBACK_TYPE calltype, ...)
{
  va_list args;
  va_start(args, calltype);
  #define   INT_ARG() va_arg(args, int)
  #define  BOOL_ARG() (Celeste_P8_bool_t)va_arg(args, int)
  #define   STR_ARG() va_arg(args, const char *)

  int ret = 0;

  switch (calltype) {
    case CELESTE_P8_SPR: {
      int tile = INT_ARG();
      int x = INT_ARG() - camera_x;
      int y = INT_ARG() - camera_y;
      int w = INT_ARG();
      int h = INT_ARG();
      int flip_x = INT_ARG();
      int flip_y = INT_ARG();
      if (w != 1 || h != 1) {
        puts("unsupported sprite scale");
        break;
      }
      if (!flip_x && !flip_y)
        spr(tile % 16, tile / 16, x, y);
      else
        spr_flippable(tile % 16, tile / 16, x, y, flip_x, flip_y);
    }

    case CELESTE_P8_BTN: {
      return !!(cur_buttons & (1u << INT_ARG()));
      break;
    }

    case CELESTE_P8_SFX: {
      int id = INT_ARG();
      printf("%d\n", id);
      // Find an appropriate channel
      do {
        last_sfx_ch = (last_sfx_ch + 1) % 4;
      } while (music_mask & (1 << last_sfx_ch));
      channels[last_sfx_ch].snd_id = id;
      channels[last_sfx_ch].note = 0;
      channels[last_sfx_ch].samples = 0;
      channels[last_sfx_ch].phase = 0;
      break;
    }

    case CELESTE_P8_PAL: {
      int c0 = INT_ARG();
      int c1 = INT_ARG();
      memcpy(pal[c0], pal_default[c1], sizeof pal[c0]);
      break;
    }

    case CELESTE_P8_PAL_RESET: {
      memcpy(pal, pal_default, sizeof pal);
      break;
    }

    case CELESTE_P8_CIRCFILL: {
      int x = INT_ARG() - camera_x;
      int y = INT_ARG() - camera_y;
      int r = INT_ARG();
      int col = INT_ARG();
      if (r > 3) {
        puts("unsupported radius");
      }
      // TODO: Replace with pixel-by-pixel calculations
      if (r == 1) {
        fillrect(x - 1, y, x + 1, y, col);
        fillrect(x, y - 1, x, y + 1, col);
      } else if (r == 2) {
        fillrect(x - 2, y - 1, x + 2, y + 1, col);
        fillrect(x - 1, y - 2, x + 1, y + 2, col);
      } else if (r == 3) {
        fillrect(x - 3, y - 1, x + 3, y + 1, col);
        fillrect(x - 2, y - 2, x + 2, y + 2, col);
        fillrect(x - 1, y - 3, x + 1, y + 3, col);
      }
      break;
    }

    case CELESTE_P8_PRINT: {
      const char *str = STR_ARG();
      int x = INT_ARG() - camera_x;
      int y = INT_ARG() - camera_y;
      int col = INT_ARG() % 16;
      for (const char *ch = str; *ch != '\0'; ch++) {
        uint32_t glyph = p8_font[*ch];
        for (int dy = 0; dy < 8; dy += 2) {
          if (glyph & (1u << 0)) pix(x + 0, y + dy + 0, col);
          if (glyph & (1u << 1)) pix(x + 1, y + dy + 0, col);
          if (glyph & (1u << 2)) pix(x + 2, y + dy + 0, col);
          if (glyph & (1u << 3)) pix(x + 3, y + dy + 0, col);
          if (glyph & (1u << 4)) pix(x + 0, y + dy + 1, col);
          if (glyph & (1u << 5)) pix(x + 1, y + dy + 1, col);
          if (glyph & (1u << 6)) pix(x + 2, y + dy + 1, col);
          if (glyph & (1u << 7)) pix(x + 3, y + dy + 1, col);
          glyph >>= 8;
        }
        x += 4;
      }
      break;
    }

    case CELESTE_P8_RECTFILL: {
      int x0 = INT_ARG() - camera_x;
      int y0 = INT_ARG() - camera_y;
      int x1 = INT_ARG() - camera_x;
      int y1 = INT_ARG() - camera_y;
      int col = INT_ARG();
      fillrect(x0, y0, x1, y1, col);
      break;
    }

    case CELESTE_P8_LINE: {
      int x0 = INT_ARG() - camera_x;
      int y0 = INT_ARG() - camera_y;
      int x1 = INT_ARG() - camera_x;
      int y1 = INT_ARG() - camera_y;
      int col = INT_ARG();
      if (x0 != x1) {
        puts("unsupported line");
        break;
      }
      for (int y = y0; y <= y1; y++) pix(x0, y, col);
      break;
    }

    case CELESTE_P8_MGET: {
      int tx = INT_ARG();
      int ty = INT_ARG();
      ret = tilemap_data[tx + ty * 128];
      break;
    }

    case CELESTE_P8_CAMERA: {
      camera_x = INT_ARG();
      camera_y = INT_ARG();
      break;
    }

    case CELESTE_P8_FGET: {
      int tile = INT_ARG();
      int flag = INT_ARG();
      ret = !!(tile_flags[tile] & (1 << flag));
      break;
    }

    case CELESTE_P8_MAP: {
      int celx = INT_ARG();
      int cely = INT_ARG();
      int sx = INT_ARG() - camera_x;
      int sy = INT_ARG() - camera_y;
      int celw = INT_ARG();
      int celh = INT_ARG();
      int layer = INT_ARG();
      for (int cy = 0; cy < celh; cy++)
      for (int cx = 0; cx < celw; cx++) {
        int tile = tilemap_data[(cely + cy) * 128 + (celx + cx)];
        if ((tile_flags[tile] & layer) == layer)
          spr(tile % 16, tile / 16, sx + cx * 8, sy + cy * 8);
      }
      break;
    }

    default:
      printf("unhandled call %d\n", (int)calltype);
  }

  va_end(args);
  return ret;
}

void p8_init()
{
  camera_x = camera_y = 0;
  for (int i = 0; i < 4; i++) channels[i].snd_id = 0xff;

  Celeste_P8_set_call_func(p8_call);
  Celeste_P8_set_rndseed(20210115);
  Celeste_P8_init();
}
