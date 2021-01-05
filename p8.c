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
#include "res/p8_gfx.h"

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
      int x = INT_ARG();
      int y = INT_ARG();
      int w = INT_ARG();
      int h = INT_ARG();
      int flip_x = INT_ARG();
      int flip_y = INT_ARG();
      if (w != 1 || h != 1) {
        puts("unsupported");
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

    case CELESTE_P8_FGET: {
      int tile = INT_ARG();
      int flag = INT_ARG();
      ret = !!(tile_flags[tile] & (1 << flag));
      break;
    }

    case CELESTE_P8_MAP: {
      int celx = INT_ARG();
      int cely = INT_ARG();
      int sx = INT_ARG();
      int sy = INT_ARG();
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

  Celeste_P8_set_call_func(p8_call);
  Celeste_P8_set_rndseed(20210115);
  Celeste_P8_init();
}
