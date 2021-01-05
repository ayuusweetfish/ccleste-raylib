#include <stdbool.h>
#include <stdio.h>

static bool a[512][512];

int main()
{
  for (int i = 0; i < 512; i++)
    for (int j = 0; j < 512; j++) {
      a[i][j] = (getchar() + getchar() + getchar() >= (0xff * 3 / 2));
      getchar();
    }

  printf("uint32_t p8_font[128] = {\n");
  for (int i = 0; i < 128; i++) {
    int y_start = (i / 16) * 32;
    int x_start = (i % 16) * 32;
    unsigned glyph = 0;
    for (int y = 0; y < 6; y++)
      for (int x = 0; x < 4; x++) {
        unsigned pix = (unsigned)a[y_start + y * 4][x_start + x * 4];
        glyph |= (pix << (y * 4 + x));
      }
    putchar(' ');
    if (i % 8 == 0) putchar(' ');
    printf("0x%08x,", glyph);
    if (i % 8 == 7) putchar('\n');
  }
  printf("};\n");

  return 0;
}
