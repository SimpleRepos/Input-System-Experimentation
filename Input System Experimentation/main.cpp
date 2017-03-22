#include "cl_Graphics.h"
#include "cl_GfxFactory.h"
#include "cl_Font.h"
#include "cl_Input.h"
#include <sstream>
#include <string>

std::wstring xin_to_s(const Input::Device::State& pad) {
  std::wstringstream ss;

  if(pad.buttons[Input::Gamepad::DPAD_UP].held) { ss << "UP"; }
  ss << "\n";
  if(pad.buttons[Input::Gamepad::DPAD_DOWN].held) { ss << "DOWN"; }
  ss << "\n";
  if(pad.buttons[Input::Gamepad::DPAD_LEFT].held) { ss << "LEFT"; }
  ss << "\n";
  if(pad.buttons[Input::Gamepad::DPAD_RIGHT].held) { ss << "RIGHT"; }
  ss << "\n";
  if(pad.buttons[Input::Gamepad::START].held) { ss << "START"; }
  ss << "\n";
  if(pad.buttons[Input::Gamepad::BACK].held) { ss << "BACK"; }
  ss << "\n";
  if(pad.buttons[Input::Gamepad::LTHUMB].held) { ss << "LTHUMB"; }
  ss << "\n";
  if(pad.buttons[Input::Gamepad::RTHUMB].held) { ss << "RTHUMB"; }
  ss << "\n";
  if(pad.buttons[Input::Gamepad::LSHOULDER].held) { ss << "LSHOULDER"; }
  ss << "\n";
  if(pad.buttons[Input::Gamepad::RSHOULDER].held) { ss << "RSHOULDER"; }
  ss << "\n";
  if(pad.buttons[Input::Gamepad::A].held) { ss << "A"; }
  ss << "\n";
  if(pad.buttons[Input::Gamepad::B].held) { ss << "B"; }
  ss << "\n";
  if(pad.buttons[Input::Gamepad::X].held) { ss << "X"; }
  ss << "\n";
  if(pad.buttons[Input::Gamepad::Y].held) { ss << "Y"; }
  ss << "\n\n";

  ss << "Left Stick: " << pad.axes[Input::Gamepad::LEFT_X] << ", " << pad.axes[Input::Gamepad::LEFT_Y] << "\n";
  ss << "Right Stick: " << pad.axes[Input::Gamepad::RIGHT_X] << ", " << pad.axes[Input::Gamepad::RIGHT_Y] << "\n";
  ss << "Left Trigger: " << pad.axes[Input::Gamepad::LTRIGGER] << "\n";
  ss << "Right Trigger: " << pad.axes[Input::Gamepad::RTRIGGER] << "\n";

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
    font.drawText(L"UP\nDOWN\nLEFT\nRIGHT\nSTART\nBACK\nLTHUMB\nRTHUMB\nLSHOULDER\nRSHOULDER\nA\nB\nX\nY", 20, 5, 5, ColorF::RED);
    font.drawText(xin_to_s(input.gamepad()), 20, 5, 5, ColorF::CYAN);
    gfx.present();
  }

  return 0;
}
