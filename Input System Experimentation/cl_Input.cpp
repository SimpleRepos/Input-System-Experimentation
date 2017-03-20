#include "cl_Input.h"
#include "cl_Window.h"
#include <array>

Input::Input(Window& win) {
  for(int& axis : axes) { axis = 0; }

  std::array<RAWINPUTDEVICE, 2> devices = {
    RAWINPUTDEVICE{ 1, 2, 0, win.getHandle() }, //mouse
    RAWINPUTDEVICE{ 1, 6, 0, win.getHandle() }  //kb
  };

  RegisterRawInputDevices(devices.data(), devices.size(), sizeof(RAWINPUTDEVICE));

  win.addProcFunc(WM_INPUT, [this](HWND hwnd, WPARAM wparam, LPARAM lparam) -> LRESULT { return procFn(hwnd, wparam, lparam); });
}

LRESULT Input::procFn(HWND hwnd, WPARAM wparam, LPARAM lparam) {
  //delegate to default proc if window is not in foreground
  if(GET_RAWINPUT_CODE_WPARAM(wparam) != 0) { return DefWindowProc(hwnd, WM_INPUT, wparam, lparam); }

  RAWINPUT rin;
  UINT size = sizeof(rin);
  GetRawInputData(reinterpret_cast<HRAWINPUT>(lparam), RID_INPUT, &rin, &size, sizeof(RAWINPUTHEADER));

  switch(rin.header.dwType) {
  case RIM_TYPEMOUSE: updateMouse(rin.data.mouse); return 0;
  case RIM_TYPEKEYBOARD: updateKeyboard(rin.data.keyboard); return 0;
  case RIM_TYPEHID: updateXInput(); break;
  }

  return DefWindowProc(hwnd, WM_INPUT, wparam, lparam);
}

void Input::updateMouse(const RAWMOUSE& data) {
  axes[MOUSE_X] += data.lLastX;
  axes[MOUSE_Y] += data.lLastX;
  //~~! these magic numbers probably mean something weird is happening - research and correct
       if(data.usButtonData ==   120) { axes[MOUSE_WHEEL]--; }
  else if(data.usButtonData == 65416) { axes[MOUSE_WHEEL]++; }

  for(short i = 0; i < numMouseButtons; i++) {
    auto iter = keyBinds.find(KeyBindRecord(MOUSE, i));
    if(iter == keyBinds.end()) { continue; }
    VKey& key = keys[iter->second];

    int val = (data.usButtonFlags >> (i * 2)) & 0b11;
    if(val & 0b01) { //pressed
      key.trigger = true;
      key.press = true;
      //~~_ key.keyDownTimeStamp = now;
    }
    if(val & 0b10) { //relesaed
      key.release = true;
      key.press = false;
    }
  }

}
