#include "cl_Graphics.h"
#include "cl_GfxFactory.h"
#include "cl_Font.h"
#include "cl_Input.h"
#include <sstream>
#include <string>


std::wstring to_s(const Input::MouseState& mouse) {
  std::wstringstream ss;

  ss << "Deltas: " << mouse.axes[Input::MouseState::DELTA_X] << ", " << mouse.axes[Input::MouseState::DELTA_Y] << ", " << mouse.axes[Input::MouseState::DELTA_WHEEL] << "\n";
  for(const auto& button : mouse.buttons) {
    ss << button.triggered << button.held << button.released << button.repeating << "\n";
  }

  return ss.str();
}

std::wstring to_s(const Input::KeyboardState& kb) {
  std::wstringstream ss;

  for(char c = 'A'; c <= 'Z'; c++) {
    if(kb.buttons[c].held) { ss << c; }
  }

  return ss.str();
}

int CALLBACK WinMain(HINSTANCE, HINSTANCE, LPSTR, int) {
  Window win("Input System", { 640, 480 });
  Graphics gfx(win);
  GfxFactory factory = gfx.createFactory();
  Font font = factory.createFont(L"Arial");
  Input input(win);

  while(win.update()) {

    Sleep(50);

    gfx.clear();
    input.update();
    font.drawText(to_s(input.mouse()), 20, 5, 5, ColorF::CYAN);
    font.drawText(to_s(input.keyboard()), 20, 200, 5, ColorF::GREEN);
    gfx.present();
  }

  return 0;
}
