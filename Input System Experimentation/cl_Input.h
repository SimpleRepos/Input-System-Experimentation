#pragma once
#include <unordered_map>
#include <vector>
#include <chrono>
#include <queue>
#include "cl_Window.h"

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
      //'held' is true if the button is currently pressed down
      bool held = false;

      //'triggered' is true if the button was not pressed but became pressed during this frame
      bool triggered = false;

      //'released' is true if the button was released during this frame
      bool released  = false;

      //'repeating' is true in the following cases:
      //  * if 'trigger' is true
      //  * one frame every 'repeatPeriodMS' AFTER the button has been held for at least 'repeatDelayMS'
      //This is useful for things like menu navigation, where the player will want precise movement from
      //pressing the button once, but may also want to hold the button in order to scroll quickly.
      bool repeating = false;
    };

    //These values can be set by the user to effect the key-repeat behavior
    unsigned int repeatDelayMS;
    unsigned int repeatPeriodMS;
    static const unsigned int DEFAULT_REPEAT_DELAY_MS  = 500;
    static const unsigned int DEFAULT_REPEAT_PERIOD_MS = 100;

    struct State {
      std::vector<Button> buttons;
      std::vector<float> axes;
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
    std::vector<float> deadZones;
    std::queue<RAWINPUT> eventQueue;

    void update(uint64_t frameTime);
    void updateXinput(uint64_t frameTime);
    void applyDeadZonedInput(int axis, int input, float axisMaxRange);
    std::function<void(const RAWINPUT&, uint64_t)> processEvent;

    void triggerButton(Button& btn, ButtonRepeatData& aux, uint64_t frameTime);
    void releaseButton(Button& btn);
    void resetButton(Button& btn);
    void updateRepeat(Button& btn, ButtonRepeatData& aux, uint64_t frameTime);

    void keyboardHandler(const RAWINPUT& rin, uint64_t frameTime);
    void mouseHandler(const RAWINPUT& rin, uint64_t frameTime);

  };

  struct Mouse {
    enum Axes    { DELTA_X, DELTA_Y, DELTA_WHEEL };
    enum Buttons { LEFT, RIGHT, WHEEL, BACK, FORWARD };
  };
  const Device::State& mouse() const { return mouseDev.state(); }

  //presently indexed by winapi VK codes
  const Device::State& keyboard() const { return kbDev.state(); }

  struct Gamepad {
    enum Axes    { LEFT_X, LEFT_Y, RIGHT_X, RIGHT_Y, LTRIGGER, RTRIGGER };
    enum Buttons { 
      DPAD_UP, DPAD_DOWN, DPAD_LEFT, DPAD_RIGHT,
      START, BACK, LTHUMB, RTHUMB,
      LSHOULDER, RSHOULDER,
      A, B, X, Y
    };
  };
  const Device::State& gamepad() const { return xinputDev.state(); }
  float getGamepadDeadZone(int axis);
  void setGamepadDeadZone(int axis, float zoneRadius);

private:
  static constexpr size_t KB_BUTTON_CT = 255;
  static constexpr size_t KB_AXIS_CT = 0;
  Device kbDev;
  static constexpr size_t MOUSE_BUTTON_CT = 5;
  static constexpr size_t MOUSE_AXIS_CT = 3;
  Device mouseDev;
  static constexpr size_t XIN_BUTTON_CT = 14;
  static constexpr size_t XIN_AXIS_CT = 6;
  Device xinputDev;


  LRESULT procFn(HWND hwnd, WPARAM wparam, LPARAM lparam);

};
