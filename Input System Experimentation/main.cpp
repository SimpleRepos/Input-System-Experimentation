#include "cl_Graphics.h"
#include "cl_GfxFactory.h"
#include "cl_Font.h"
#include "cl_Input.h"
#include <sstream>
#include <string>


std::wstring m_to_s(const Input::Device::State& mouse) {
  std::wstringstream ss;

  ss << "Deltas: " << mouse.axes[Input::MouseAxes::DELTA_X] << ", " << mouse.axes[Input::MouseAxes::DELTA_Y] << ", " << mouse.axes[Input::MouseAxes::DELTA_WHEEL] << "\n";
  for(const auto& button : mouse.buttons) {
    ss << button.triggered << button.held << button.released << button.repeating << "\n";
  }

  return ss.str();
}

std::wstring kb_to_s(const Input::Device::State& kb) {
  std::wstringstream ss;

  for(char c = 'A'; c <= 'Z'; c++) {
    if(kb.buttons[c].held) { ss << c; }
  }

  return ss.str();
}

std::wstring xin_to_s(const Input::Device::State& pad) {
  std::wstringstream ss;

  if(pad.buttons[0].repeating) { ss << "up\n"; }
  if(pad.buttons[1].repeating) { ss << "dn\n"; }
  if(pad.buttons[2].repeating) { ss << "lt\n"; }
  if(pad.buttons[3].repeating) { ss << "rt\n"; }

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
    font.drawText(m_to_s(input.mouse()), 20, 5, 5, ColorF::CYAN);
    font.drawText(kb_to_s(input.keyboard()), 20, 200, 5, ColorF::GREEN);
    font.drawText(xin_to_s(input.gamepad()), 20, 200, 200, ColorF::RED);
    gfx.present();
  }

  return 0;
}
