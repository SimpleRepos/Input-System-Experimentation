#include "cl_Graphics.h"
#include "cl_GfxFactory.h"
#include "cl_Font.h"
#include "cl_Input.h"
#include <sstream>
#include <string>

std::wstring to_s(const Input::Device::State& state) {
  std::wstringstream ss;

  constexpr int columns = 16;
  const int stride = state.buttons.size() / columns;
  for(size_t x = 0; x < state.buttons.size(); ) {
    for(int i = 0; i < columns; i++) {
      if(++x >= state.buttons.size()) { break; }
      if(state.buttons[x].held) { ss <<  "O"; }
      else                      { ss <<  "."; }
      ss <<  " ";
    }
    ss << "\n";
  }

  return ss.str();
}

int CALLBACK WinMain(HINSTANCE, HINSTANCE, LPSTR, int) {
  Window win("Input System", { 800, 600 });
  Graphics gfx(win);
  GfxFactory factory = gfx.createFactory();
  Font font = factory.createFont(L"Courier New");
  Input input(win);

  while(win.update()) {
    Sleep(25);

    gfx.clear();
    input.update();
    font.drawText(to_s(input.keyboard()), 10, 5, 5, ColorF::CYAN);
    gfx.present();
  }

  return 0;
}
