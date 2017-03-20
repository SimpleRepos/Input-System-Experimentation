#pragma once
#include <unordered_map>
#include <string>
#include <sstream>
#include <array>
#include "cl_Window.h"

// RIN https://msdn.microsoft.com/en-us/library/windows/desktop/ms645543(v=vs.85).aspx
// XIN https://msdn.microsoft.com/en-us/library/windows/desktop/ee417001(v=vs.85).aspx

class Input {
public:
  Input(Window& win);

  enum DeviceType: uint16_t {
    INVALID,
    MOUSE,
    KEYBOARD,
    XIN
  };

  struct KeyBindRecord {
    KeyBindRecord(DeviceType dev, short keyCode) : value(keyCode + (static_cast<uint32_t>(dev) << 16)) {}
    const uint32_t value;

    DeviceType  type() const { return static_cast<DeviceType>(value >> 16); }
    uint16_t keyCode() const { return static_cast<unsigned short>(value && 16); }

    operator uint32_t() const { return value; }
  };

  void addKey(const std::string& name) { keys[name]; }
  void bindKey(const std::string& name, KeyBindRecord key) { keyBinds[key] = name; }

  void removeKey(const std::string& name);
  void unbindKey(KeyBindRecord key);
  void resetKeyBinds();

  void addAxis();
  void removeAxis();
  void bindAxis();
  void unbindAxis();
  void resetAxisbinds();

  bool trigger(const std::string& key) const;
  bool press(const std::string& key) const { return keys.at(key).press; }
  bool release(const std::string& key) const;
  int axis(const std::string& axis) const;

  //~~_ repeat
  //~~_ dead zones

private:
  enum Axis {
    MOUSE_X,
    MOUSE_Y,
    MOUSE_WHEEL,
    PAD_LX,
    PAD_LY,
    PAD_RX,
    PAD_RY,
    NUM_AXES
  };

  std::array<int, NUM_AXES> axes;

  struct VKey {
    unsigned int keyDownTimeStamp;
    bool trigger, press, release;
  };

  static const int numMouseButtons = 5; //~~! detect

  std::unordered_map<std::string, VKey> keys;
  std::unordered_map<uint32_t, std::string> keyBinds;
  //std::unordered_map<std::string, int> axes;
  std::unordered_map<Axis, std::string> axisBinds;

  LRESULT procFn(HWND hwnd, WPARAM wparam, LPARAM lparam);
  void updateMouse(const RAWMOUSE& data);
  void updateKeyboard(const RAWKEYBOARD& data) {} //~~_
  void updateXInput() {} //~~_

};

