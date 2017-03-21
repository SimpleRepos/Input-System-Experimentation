#pragma once
#include <unordered_map>
#include <array>
#include <chrono>
#include <queue>
#include "cl_Window.h"

//XIN https://msdn.microsoft.com/en-us/library/windows/desktop/ee417001(v=vs.85).aspx
//~~_ xinput with dead zones

class Input {
public:
  struct Button {
    bool held      = false; //is the button currently down
    bool triggered = false; //is this the first frame of it being pressed
    bool released  = false; //was it released during this frame
    bool repeating = false; //~~# true on trigger frame, then after repeatDelayMS every repeatPeriodMS
  };

  //These values can be set by the user to effect the key-repeat behavior
  unsigned int repeatDelayMS;
  unsigned int repeatPeriodMS;
  static const unsigned int DEFAULT_REPEAT_DELAY_MS  = 500;
  static const unsigned int DEFAULT_REPEAT_PERIOD_MS = 100;

  struct MouseState {
    static const size_t BUTTON_CT = 5;
    std::array<Button, BUTTON_CT> buttons;
    static const size_t AXIS_CT = 3;
    enum AXIS { DELTA_X, DELTA_Y, DELTA_WHEEL };
    std::array<int, AXIS_CT> axes;
  };

  struct KeyboardState {
    static const size_t BUTTON_CT = 255;
    std::array<Button, BUTTON_CT> buttons;
    //static const size_t AXIS_CT = 0;
    //std::array<int, AXIS_CT> axes;
  };

  Input(Window& win);
  void update();
  const MouseState& mouse() const { return mouseState; }
  const KeyboardState& keyboard() const { return kbState; }

private:
  struct ButtonRepeatData {
    uint64_t triggerTimeMS;
    unsigned int repeatPrev;
  };

  MouseState mouseState;
  std::array<ButtonRepeatData, MouseState::BUTTON_CT> mouseAux;
  std::queue<RAWMOUSE> mouseEvents;

  KeyboardState kbState;
  std::array<ButtonRepeatData, KeyboardState::BUTTON_CT> kbAux;
  std::queue<RAWKEYBOARD> kbEvents;

  LRESULT procFn(HWND hwnd, WPARAM wparam, LPARAM lparam);

  void triggerButton(Button& btn, ButtonRepeatData& aux, uint64_t frameTime);
  void releaseButton(Button& btn);
  void resetButton(Button& btn);
  void updateRepeat(Button& btn, ButtonRepeatData& aux, uint64_t frameTime);

  void updateMouseState(uint64_t frameTime);
  void processMouseEvent(const RAWMOUSE& event, uint64_t frameTime);

  void updateKeyboardState(uint64_t frameTime);
  void processKeyboardEvent(const RAWKEYBOARD& event, uint64_t frameTime);

};
