#include "p8.h"

#include <stdint.h>
#include <string.h>

static uint8_t fb[P8_SCR_SIZE][P8_SCR_SIZE][4];

void *p8_draw()
{
  return (void *)&fb[0][0][0];
}

void p8_update()
{
  memset(fb, 0, sizeof fb);
}
