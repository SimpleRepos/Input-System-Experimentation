#pragma once
#include <Windows.h>
#include <string>
#include <unordered_map>
#include <functional>

class Window {
public:
  struct Dimensions {
    LONG width;
    LONG height;
  };

  Window(const std::string& windowTitle, Dimensions userAreaDims);
  ~Window();

  //Objects of this type cannot be moved or copied.
  //The address of the object must remain static after declaration.
  Window(Window&& other) = delete;
  Window& operator=(Window&&) = delete;
  Window(const Window&) = delete;
  void operator=(const Window&) = delete;

  void addProcFunc(UINT message, std::function<LRESULT(HWND, WPARAM, LPARAM)> func);
  void clearProcFunc(UINT message);
  bool update();

  HWND getHandle();

  const Dimensions USER_AREA_DIMS;
  const Dimensions WINDOW_DIMS;

private:
  HWND handle;
  std::string cName;

  static LRESULT CALLBACK WindowProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
  std::unordered_map<WPARAM, std::function<void(HWND, LPARAM)>> keyFuncs;
  std::unordered_map<UINT, std::function<LRESULT(HWND, WPARAM, LPARAM)>> procFuncs;

  Dimensions calcWinDims(Dimensions userDims);

  static const long STYLE = WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX | WS_VISIBLE;

};
