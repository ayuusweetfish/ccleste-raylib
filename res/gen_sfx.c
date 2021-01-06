#include "p8_sfx_defs.h"
#include <stdio.h>

static inline unsigned getuint4()
{
  char a = getchar();
  if (a >= '0' && a <= '9') return (unsigned)(a - '0');
  else return (unsigned)(a - 'a' + 10);
}
static inline unsigned getuint8()
{
  unsigned hi = getuint4();
  return (hi << 4) | getuint4();
}

int main()
{
  // Sound effects
  printf("static const p8_snd p8_sfx[64] = {\n");
  for (int i = 0; i < 64; i++) {
    if (i == 0) printf("  {\n");
    getuint8();

    unsigned spd = getuint8();
    unsigned lpstart = getuint8();
    unsigned lpend = getuint8();
    printf("    .spd = %u, .lpstart = %u, .lpend = %u, .notes = {\n",
      spd, lpstart, lpend);

    for (int j = 0; j < 32; j++) {
      unsigned pitch = getuint8();
      unsigned wform = getuint4();
      unsigned vol = getuint4();
      unsigned eff = getuint4();
      printf("      { .pitch = %2u, .wform = %u, .vol = %u, .eff = %u },\n",
        pitch, wform, vol, eff);
    }
    printf("    }\n");

    getchar();  // Newline
    if (i != 63) printf("  }, {\n");
    else printf("  }\n");
  }
  printf("};\n");

  return 0;
}
