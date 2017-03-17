#include "cl_Window.h"
#include <vector>
#include <string>
#include <sstream>
#include "cl_Graphics.h"
#include "cl_GfxFactory.h"
#include "cl_Font.h"

// RIN https://msdn.microsoft.com/en-us/library/windows/desktop/ms645543(v=vs.85).aspx
// XIN https://msdn.microsoft.com/en-us/library/windows/desktop/ee417001(v=vs.85).aspx

class Input {
public:
  Input(Window& win) : state({}) {
    RAWINPUTDEVICE mouse = { 1, 2, 0, win.getHandle() };
    RegisterRawInputDevices(&mouse, 1, sizeof(RAWINPUTDEVICE));

    win.addProcFunc(WM_INPUT, [this](HWND hwnd, WPARAM wparam, LPARAM lparam) -> LRESULT { return procFn(hwnd, wparam, lparam); });
  }

  struct MouseState {
    bool buttons[5];
    int x, y;
    int scroll;

    operator std::wstring() const {
      std::wstringstream ss;

      ss << "Buttons: ";
      for(auto b : buttons) { ss << b ? 1 : 0; }
      ss << "\nX: " << x;
      ss << "\nY: " << y;
      ss << "\nScroll: " << scroll;

      return ss.str();
    }
  } state;

private:
  LRESULT procFn(HWND hwnd, WPARAM wparam, LPARAM lparam) {
    if(GET_RAWINPUT_CODE_WPARAM(wparam) != 0) { return DefWindowProc(hwnd, WM_INPUT, wparam, lparam); }

    RAWINPUT rin;
    UINT size = sizeof(rin);
    GetRawInputData(reinterpret_cast<HRAWINPUT>(lparam), RID_INPUT, &rin, &size, sizeof(RAWINPUTHEADER));

    if(rin.header.dwType == RIM_TYPEMOUSE) {
      updateMouse(rin.data.mouse);
      return 0;
    }

    return DefWindowProc(hwnd, WM_INPUT, wparam, lparam);
  }

  void updateMouse(const RAWMOUSE& data) {
    if(data.usButtonFlags & 0b0000000001) { state.buttons[0] = true;  }
    if(data.usButtonFlags & 0b0000000010) { state.buttons[0] = false; }
    if(data.usButtonFlags & 0b0000000100) { state.buttons[1] = true;  }
    if(data.usButtonFlags & 0b0000001000) { state.buttons[1] = false; }
    if(data.usButtonFlags & 0b0000010000) { state.buttons[2] = true;  }
    if(data.usButtonFlags & 0b0000100000) { state.buttons[2] = false; }
    if(data.usButtonFlags & 0b0001000000) { state.buttons[3] = true;  }
    if(data.usButtonFlags & 0b0010000000) { state.buttons[3] = false; }
    if(data.usButtonFlags & 0b0100000000) { state.buttons[4] = true;  }
    if(data.usButtonFlags & 0b1000000000) { state.buttons[4] = false; }

    state.x += data.lLastX;
    state.y += data.lLastY;

    if(data.usButtonData == 120) { state.scroll--; }
    else if(data.usButtonData == 65416) { state.scroll++; }
  }

};

int CALLBACK WinMain(HINSTANCE, HINSTANCE, LPSTR, int) {
  Window win("Input System", { 640, 480 });
  Graphics gfx(win);
  GfxFactory factory = gfx.createFactory();
  Font font = factory.createFont(L"Arial");
  Input input(win);

  while(win.update()) {
    gfx.clear();
    font.drawText(input.state, 12, 5, 5, ColorF::CYAN);
    gfx.present();
  }

  return 0;
}
