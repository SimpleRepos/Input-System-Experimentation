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

  struct DeviceButton {
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

  struct DeviceState {
    std::vector<DeviceButton> buttons;
    std::vector<float> axes;
  };

  struct Mouse {
    enum Axes    { DELTA_X, DELTA_Y, DELTA_WHEEL };
    enum Buttons { L_BUTTON, R_BUTTON, WHEEL_BUTTON, BACK, FORWARD };
  };
  const DeviceState& mouse() const { return mouseDev.state(); }

  //presently indexed by winapi VK codes
  const DeviceState& keyboard() const { return kbDev.state(); }

  struct Gamepad {
    enum Axes    {
      LEFT_X, LEFT_Y,
      RIGHT_X, RIGHT_Y,
      LTRIGGER, RTRIGGER
    };
    enum Buttons { 
      DPAD_UP, DPAD_DOWN, DPAD_LEFT, DPAD_RIGHT,
      START, BACK, 
      LTHUMB, RTHUMB,
      LSHOULDER, RSHOULDER,
      A, B, X, Y
    };
  };
  float getGamepadDeadZone(int axis);
  void setGamepadDeadZone(int axis, float zoneRadius);
  const DeviceState& gamepad() const { return xinputDev.state(); }

  //the user may change these values to customize the DeviceButton repeat behavior
  unsigned int repeatDelayMS;
  unsigned int repeatPeriodMS;
  static const unsigned int DEFAULT_REPEAT_DELAY_MS  = 500;
  static const unsigned int DEFAULT_REPEAT_PERIOD_MS = 100;

private:
  class Device {
  public:
    enum class Type { MOUSE, KEYBOARD, XINPUT };
    const Type type;

    Device(size_t buttonCt, size_t axisCt, Type type);

    void update(uint64_t frameTime, const Input& input);
    const DeviceState& state() const { return devState; }

    std::vector<float> deadZones;

    void enqueueEvent(const RAWINPUT& event) { eventQueue.push(event); }

  private:
    struct ButtonRepeatData {
      uint64_t triggerTimeMS;
      unsigned int repeatPrev;
    };

    DeviceState devState;
    std::vector<ButtonRepeatData> repeatData;
    std::vector<DeviceButton> xinputPrev;
    std::queue<RAWINPUT> eventQueue;

    void updateXinput(uint64_t frameTime);
    void applyDeadZonedInput(int axis, int input, float axisMaxRange);
    std::function<void(const RAWINPUT&, uint64_t)> processEvent;

    void triggerButton(DeviceButton& btn, ButtonRepeatData& aux, uint64_t frameTime);
    void releaseButton(DeviceButton& btn);
    void resetButton(DeviceButton& btn);
    void updateRepeat(DeviceButton& btn, ButtonRepeatData& aux, uint64_t frameTime, const Input& input);

    void keyboardHandler(const RAWINPUT& rin, uint64_t frameTime);
    void mouseHandler(const RAWINPUT& rin, uint64_t frameTime);

  };

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
