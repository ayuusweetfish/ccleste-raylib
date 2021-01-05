#include <stdio.h>

static char a[64][128];

int main()
{
  for (int y = 0; y < 64; y++) {
    for (int x = 0; x < 128; x++)
      a[y][x] = getchar();
    getchar();
  }

  printf("static const uint32_t p8_gfx[8][16][8] = {\n");
  for (int y = 0; y < 8; y++) {
    if (y == 0) printf("  {\n");
    for (int x = 0; x < 16; x++) {
      printf("    {");
      for (int dy = 0; dy < 8; dy++) {
        printf("0x");
        for (int dx = 7; dx >= 0; dx--)
          putchar(a[y * 8 + dy][x * 8 + dx]);
        if (dy != 7) printf(", ");
        else printf("},\n");
      }
    }
    if (y != 7) printf("  }, {\n");
    else printf("  }\n");
  }
  printf("};\n");

  return 0;
}
