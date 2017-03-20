#include <vector>
#include "cl_Graphics.h"
#include "cl_GfxFactory.h"
#include "cl_Font.h"
#include "cl_Input.h"


int CALLBACK WinMain(HINSTANCE, HINSTANCE, LPSTR, int) {
  Window win("Input System", { 640, 480 });
  Graphics gfx(win);
  GfxFactory factory = gfx.createFactory();
  Font font = factory.createFont(L"Arial");
  Input input(win);

  input.addKey("derp");
  input.bindKey("derp", Input::KeyBindRecord(Input::MOUSE, 1));

  while(win.update()) {
    gfx.clear();
    bool pressed = input.press("derp");
    font.drawText(pressed ? L"yes" : L"no", 20, 5, 5, ColorF::CYAN);
    gfx.present();
  }

  return 0;
}
