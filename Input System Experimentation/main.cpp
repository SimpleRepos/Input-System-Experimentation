#include "cl_Window.h"
#include <vector>
#include <string>
#include <sstream>
#include "cl_Graphics.h"
#include "cl_GfxFactory.h"
#include "cl_Font.h"

// RIN https://msdn.microsoft.com/en-us/library/windows/desktop/ms645543(v=vs.85).aspx
// XIN https://msdn.microsoft.com/en-us/library/windows/desktop/ee417001(v=vs.85).aspx

LRESULT inputHandler(HWND hwnd, WPARAM wparam, LPARAM lparam, Font& font, Graphics& gfx) {
  if(GET_RAWINPUT_CODE_WPARAM(wparam)) { return DefWindowProc(hwnd, WM_INPUT, wparam, lparam); }

  RAWINPUTHEADER header;
  UINT size = sizeof(RAWINPUTHEADER);
  GetRawInputData(reinterpret_cast<HRAWINPUT>(lparam), RID_HEADER, &header, &size, size);
  if(header.dwType != RIM_TYPEMOUSE) { return DefWindowProc(hwnd, WM_INPUT, wparam, lparam); }

  RAWINPUT rin;
  size = sizeof(rin);
  GetRawInputData(reinterpret_cast<HRAWINPUT>(lparam), RID_INPUT, &rin, &size, sizeof(RAWINPUTHEADER));
  RAWMOUSE mouse = rin.data.mouse;
  
  std::wstringstream ss;
  ss << "Flags: " << mouse.usFlags << "\nButton Flags: " << mouse.usButtonFlags << "\n\nX: " << mouse.lLastX << "\nY: " << mouse.lLastY << "\nWheel: " << mouse.usButtonData;
  gfx.clear();
  font.drawText(ss.str(), 12, 5, 5, ColorF::CYAN);

  return 0;
}

int CALLBACK WinMain(HINSTANCE, HINSTANCE, LPSTR, int) {
  Window win("Input System", { 640, 480 });
  Graphics gfx(win);
  GfxFactory factory = gfx.createFactory();
  Font font = factory.createFont(L"Arial");

  RAWINPUTDEVICE mouse;
  mouse.usUsagePage = 0x01;
  mouse.usUsage = 0x02;
  mouse.dwFlags = 0;
  mouse.hwndTarget = win.getHandle();
  RegisterRawInputDevices(&mouse, 1, sizeof(RAWINPUTDEVICE));

  win.addProcFunc(WM_INPUT, [&gfx, &font](HWND hwnd, WPARAM wparam, LPARAM lparam) -> LRESULT { return inputHandler(hwnd, wparam, lparam, font, gfx); });
  while(true) {
    if(!win.update()) { break; }
    gfx.present();
  }

  return 0;
}
