#include "raylib.h"

#define WIN_SIZE  256

int main()
{
  InitWindow(WIN_SIZE, WIN_SIZE, "Window");
  SetTargetFPS(60);

  while (!WindowShouldClose()) {
    BeginDrawing();
    ClearBackground(WHITE);
    EndDrawing();
  }

  CloseWindow();
  return 0;
}
