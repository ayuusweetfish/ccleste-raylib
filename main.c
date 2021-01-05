#include "p8.h"
#include "raylib.h"

#define WIN_SIZE  512

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
  free(data);

  p8_init();

  while (!WindowShouldClose()) {
    BeginDrawing();
    ClearBackground(BLACK);

    unsigned buttons = 0;
    if (IsKeyPressed(KEY_LEFT))   buttons |= P8_BTN_L;
    if (IsKeyPressed(KEY_RIGHT))  buttons |= P8_BTN_R;
    if (IsKeyPressed(KEY_UP))     buttons |= P8_BTN_U;
    if (IsKeyPressed(KEY_DOWN))   buttons |= P8_BTN_D;
    if (IsKeyPressed(KEY_Z) || IsKeyPressed(KEY_C) ||
        IsKeyPressed(KEY_N))      buttons |= P8_BTN_O;
    if (IsKeyPressed(KEY_X) || IsKeyPressed(KEY_V) ||
        IsKeyPressed(KEY_M))      buttons |= P8_BTN_X;
    p8_update(buttons);

    // Update and draw texture
    UpdateTexture(tex, p8_draw());
    DrawTextureEx(tex, (Vector2) {0, 0}, 0,
      (float)WIN_SIZE / P8_SCR_SIZE, WHITE);

    EndDrawing();
  }

  CloseWindow();
  return 0;
}
