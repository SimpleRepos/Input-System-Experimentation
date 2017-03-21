#pragma once
#include <unordered_map>
#include <vector>
#include <chrono>
#include <queue>
#include "cl_Window.h"

//~~_ dead zones

class Input {
public:
  Input(Window& win);
  void update();

  class Device {
  public:
    enum class Type { MOUSE, KEYBOARD, XINPUT };
    Device(size_t buttonCt, size_t axisCt, Type type);

    const Type type;

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

    struct State {
      std::vector<Button> buttons;
      std::vector<int> axes;
    };

    const State& state() const { return devState; }

  private:
    friend class Input;

    struct ButtonRepeatData {
      uint64_t triggerTimeMS;
      unsigned int repeatPrev;
    };

    State devState;
    std::vector<ButtonRepeatData> repeatData;
    std::vector<Button> xinputPrev;
    std::queue<RAWINPUT> eventQueue;

    void update(uint64_t frameTime);
    std::function<void(const RAWINPUT&, uint64_t)> processEvent;
    void triggerButton(Button& btn, ButtonRepeatData& aux, uint64_t frameTime);
    void releaseButton(Button& btn);
    void resetButton(Button& btn);
    void updateRepeat(Button& btn, ButtonRepeatData& aux, uint64_t frameTime);

    void keyboardHandler(const RAWINPUT& rin, uint64_t frameTime);
    void mouseHandler(const RAWINPUT& rin, uint64_t frameTime);

  };

  enum MouseAxes { DELTA_X, DELTA_Y, DELTA_WHEEL };
  const Device::State& mouse() const { return mouseDev.state(); }

  const Device::State& keyboard() const { return kbDev.state(); }

  enum GamepadAxes { LEFT_X, LEFT_Y, RIGHT_X, RIGHT_Y, LTRIGGER, RTRIGGER };
  const Device::State& gamepad() const { return xinputDev.state(); }

private:
  Device kbDev;
  Device mouseDev;
  Device xinputDev;

  LRESULT procFn(HWND hwnd, WPARAM wparam, LPARAM lparam);

};
