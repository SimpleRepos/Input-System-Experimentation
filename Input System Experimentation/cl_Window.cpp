#include "cl_Window.h"

Window::Window(const std::string& title, Dimensions userAreaDims) : 
  USER_AREA_DIMS(userAreaDims), 
  WINDOW_DIMS(calcWinDims(userAreaDims)),
  cName(title.c_str())
{
  //note that this WNDCLASS instructs the OS to allocate 8 bytes of metadata storage for the window
  WNDCLASS wc = {
    CS_HREDRAW|CS_VREDRAW, WindowProc, 0, 8, GetModuleHandle(NULL), 0,
    LoadCursor(NULL, IDC_ARROW), (HBRUSH)COLOR_WINDOW, 0, cName.c_str()
  };
  if(!RegisterClass(&wc)) { throw std::runtime_error("Failed to register window class."); }

  handle = CreateWindow(cName.c_str(), title.c_str(), STYLE, CW_USEDEFAULT, CW_USEDEFAULT, WINDOW_DIMS.width, WINDOW_DIMS.height, 0, 0, wc.hInstance, 0);

  //store a pointer to this object in the OS window's metadata storage
  SetLastError(0);
  SetWindowLongPtr(handle, 0, LONG_PTR(this));
  if(GetLastError()) { throw std::runtime_error("Failed to backlink Window object to HWND."); }

  addProcFunc(WM_DESTROY, [](HWND, WPARAM, LPARAM) -> LRESULT { PostQuitMessage(0); return 0; });
}

Window::~Window() {
  if(!cName.empty()) {
    UnregisterClass(cName.c_str(), GetModuleHandle(NULL));
  }
}

void Window::addProcFunc(UINT message, std::function<LRESULT(HWND, WPARAM, LPARAM)> func) {
  procFuncs[message] = func;
}

void Window::clearProcFunc(UINT message) {
  procFuncs.erase(message);
}

bool Window::update() {
  MSG msg;
  while(PeekMessage(&msg, 0, 0, 0, PM_REMOVE)) {
    //TranslateMessage(&msg);
    DispatchMessage(&msg);
    if(msg.message == WM_QUIT) { return false; }
  }

  return true;
}

HWND Window::getHandle() {
  return handle;
}

LRESULT CALLBACK Window::WindowProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) 	{
  Window* pWindow = reinterpret_cast<Window*>(GetWindowLongPtr(hWnd, 0));

  if(pWindow) {
    if(pWindow->procFuncs.count(message)) {
      return pWindow->procFuncs[message](hWnd, wParam, lParam);
    }
  }

  return DefWindowProc(hWnd, message, wParam, lParam);
}

Window::Dimensions Window::calcWinDims(Dimensions userDims) {
  RECT rect{ 0, 0, userDims.width, userDims.height };
  AdjustWindowRect(&rect, STYLE, FALSE);
  return Dimensions{ rect.right - rect.left, rect.bottom - rect.top };
}

