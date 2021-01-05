#include "p8.h"

#include <stddef.h>
#include "ccleste/celeste.h"

#include <stdarg.h>
#include <stdint.h>
#include <string.h>

static uint8_t fb[P8_SCR_SIZE][P8_SCR_SIZE][4];

void *p8_draw()
{
  puts("draw");
  memset(fb, 0, sizeof fb);
  Celeste_P8_draw();
  return (void *)&fb[0][0][0];
}

void p8_update()
{
  puts("update");
  Celeste_P8_update();
}

// Interface to ccleste

#include "ccleste/tilemap.h"

static int camera_x, camera_y;

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
      // TODO
      return 0;
      break;
    }

    case CELESTE_P8_PAL_RESET: {
      // TODO
      break;
    }

    case CELESTE_P8_RECTFILL: {
      // TODO
      int x0 = INT_ARG() - camera_x;
      int y0 = INT_ARG() - camera_y;
      int x1 = INT_ARG() - camera_x;
      int y1 = INT_ARG() - camera_y;
      int col = INT_ARG();
      break;
    }

    case CELESTE_P8_PRINT: {
      const char *str = STR_ARG();
      int x = INT_ARG() - camera_x;
      int y = INT_ARG() - camera_y;
      int col = INT_ARG() % 16;
      puts(str);
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
