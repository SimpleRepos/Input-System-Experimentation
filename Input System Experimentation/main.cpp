#include "cl_Graphics.h"
#include "cl_GfxFactory.h"
#include "cl_Font.h"
#include "cl_Input.h"
#include <sstream>
#include <string>

std::wstring to_s(const Input::Device::State& state) {
  std::wstringstream ss;

  ss << "Position Delta: " << state.axes[Input::Mouse::Axes::DELTA_X] << ", " << state.axes[Input::Mouse::Axes::DELTA_Y] << "\n";
  ss << "Wheel Delta:" << state.axes[Input::Mouse::Axes::DELTA_WHEEL] << "\n";
  ss << "Left Button: "    << state.buttons[Input::Mouse::Buttons::LEFT].held << "\n";
  ss << "Right Button: "   << state.buttons[Input::Mouse::Buttons::RIGHT].held << "\n";
  ss << "Wheel Button: "   << state.buttons[Input::Mouse::Buttons::WHEEL].held << "\n";
  ss << "Back Button: "    << state.buttons[Input::Mouse::Buttons::BACK].held << "\n";
  ss << "Forward Button: " << state.buttons[Input::Mouse::Buttons::FORWARD].held << "\n";

  return ss.str();
}

int CALLBACK WinMain(HINSTANCE, HINSTANCE, LPSTR, int) {
  Window win("Input System", { 640, 480 });
  Graphics gfx(win);
  GfxFactory factory = gfx.createFactory();
  Font font = factory.createFont(L"Arial");
  Input input(win);

  while(win.update()) {
    Sleep(25);

    gfx.clear();
    input.update();
    font.drawText(to_s(input.mouse()), 20, 5, 5, ColorF::CYAN);
    gfx.present();
  }

  return 0;
}
