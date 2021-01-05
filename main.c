#include "p8.h"
#include "raylib.h"

#define WIN_SIZE  256

int main()
{
  InitWindow(WIN_SIZE, WIN_SIZE, "Window");
  SetTargetFPS(60);

  // Create empty texture by loading empty texture data,
  // as raylib does not provide a more convenient API
  void *data = malloc(P8_SCR_SIZE * P8_SCR_SIZE * 4);
  Texture2D tex = LoadTextureFromImage((Image) {
    .data = data,
    .width = P8_SCR_SIZE,
    .height = P8_SCR_SIZE,
    .mipmaps = 1,
    .format = UNCOMPRESSED_R8G8B8A8
  });

  while (!WindowShouldClose()) {
    BeginDrawing();
    ClearBackground(BLACK);

    p8_update();

    // Update and draw texture
    UpdateTexture(tex, p8_draw());
    DrawTextureEx(tex, (Vector2) {0, 0}, 0,
      (float)WIN_SIZE / P8_SCR_SIZE, WHITE);

    EndDrawing();
  }

  CloseWindow();
  return 0;
}
