#include "p8.h"

#include <stddef.h>
#include "ccleste/celeste.h"

#include <stdarg.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

static uint8_t fb[P8_SCR_SIZE][P8_SCR_SIZE][4];
static unsigned cur_buttons;

void *p8_draw()
{
  puts("draw");
  memset(fb, 0, sizeof fb);
  Celeste_P8_draw();
  return (void *)&fb[0][0][0];
}

void p8_update(unsigned buttons)
{
  puts("update");
  cur_buttons = buttons;
  Celeste_P8_update();
}

// Interface to ccleste

#include "ccleste/tilemap.h"
#include "res/p8_font.h"

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

static int p8_call(CELESTE_P8_CALLBACK_TYPE calltype, ...)
{
  va_list args;
  va_start(args, calltype);
  #define   INT_ARG() va_arg(args, int)
  #define  BOOL_ARG() (Celeste_P8_bool_t)va_arg(args, int)
  #define   STR_ARG() va_arg(args, const char *)

  int ret = 0;

  switch (calltype) {
    case CELESTE_P8_BTN: {
      return !!(cur_buttons & (1u << INT_ARG()));
      break;
    }

    case CELESTE_P8_PAL_RESET: {
      memcpy(pal, pal_default, sizeof pal);
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
      for (int y = y0; y <= y1; y++)
        for (int x = x0; x <= x1; x++)
          pix(x, y, col);
      break;
    }

    case CELESTE_P8_MGET: {
      int tx = INT_ARG();
      int ty = INT_ARG();
      ret = tilemap_data[tx + ty * 128];
      break;
    }

    case CELESTE_P8_MAP: {
      // TODO
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

  Celeste_P8_set_call_func(p8_call);
  Celeste_P8_set_rndseed(20210115);
  Celeste_P8_init();
}
