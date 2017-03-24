#include "cl_Graphics.h"
#include "cl_GfxFactory.h"
#include "cl_Font.h"
#include "cl_Input.h"
#include <sstream>
#include <string>

void appendButton(std::wstringstream& stream, const Input::DeviceButton& button) {
  stream <<  "[";
  stream << (button.triggered ? "O" : ".");
  stream << (button.held      ? "O" : ".");
  stream << (button.released  ? "O" : ".");
  stream << (button.repeating ? "O" : ".");
  stream <<  "] ";
}

std::wstring k_to_s(const Input::DeviceState& state) {
  std::wstringstream ss;

  constexpr int columns = 16;
  const int stride = state.buttons.size() / columns;
  for(size_t x = 0; x < state.buttons.size(); ) {
    for(int i = 0; i < columns; i++) {
      if(++x >= state.buttons.size()) { break; }
      appendButton(ss, state.buttons[x]);
    }
    ss << "\n";
  }

  return ss.str();
}

std::wstring m_to_s(const Input::DeviceState& state) {
  std::wstringstream ss;

  ss << "Delta Pos: " << state.axes[Input::Mouse::Axes::DELTA_X] << ", " << state.axes[Input::Mouse::Axes::DELTA_Y] << ", " << state.axes[Input::Mouse::Axes::DELTA_WHEEL] << "\n";
  ss << "Buttons: ";
  for(auto& button : state.buttons) { appendButton(ss, button); }

  return ss.str();
}

std::wstring g_to_s(const Input::DeviceState& state) {
  std::wstringstream ss;

  ss << "Axes: ";
  for(float axis : state.axes) { ss << axis << "\n"; }

  ss << "Buttons: ";
  for(auto& button : state.buttons) { appendButton(ss, button); }

  return ss.str();
}

int CALLBACK WinMain(HINSTANCE, HINSTANCE, LPSTR, int) {
  Window win("Input System", { 800, 600 });
  Graphics gfx(win);
  GfxFactory factory = gfx.createFactory();
  Font font = factory.createFont(L"Courier New");
  Input input(win);

  while(win.update()) {
    Sleep(50);

    gfx.clear();
    input.update();
    font.drawText(k_to_s(input.keyboard()), 10, 5, 5, ColorF::CYAN);
    font.drawText(m_to_s(input.mouse()), 10, 5, 200, ColorF::MAGENTA);
    font.drawText(g_to_s(input.gamepad()), 10, 5, 235, ColorF::YELLOW);
    gfx.present();
  }

  return 0;
}
