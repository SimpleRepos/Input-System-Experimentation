#include "cl_Input.h"
#include <Xinput.h>

#pragma comment(lib, "Xinput9_1_0.lib")

Input::Input(Window& win) : repeatDelayMS(DEFAULT_REPEAT_DELAY_MS), repeatPeriodMS(DEFAULT_REPEAT_PERIOD_MS) {
  constexpr size_t NUM_RIN_DEVICES = 2;
  RAWINPUTDEVICE devices[NUM_RIN_DEVICES] = {
    RAWINPUTDEVICE{ 1, 2, 0, win.getHandle() }, //mouse
    RAWINPUTDEVICE{ 1, 6, 0, win.getHandle() }  //kb
  };

  RegisterRawInputDevices(devices, NUM_RIN_DEVICES, sizeof(RAWINPUTDEVICE));

  win.addProcFunc(WM_INPUT, [this](HWND hwnd, WPARAM wparam, LPARAM lparam) -> LRESULT { return procFn(hwnd, wparam, lparam); });
}

void Input::update() {
  auto now = std::chrono::high_resolution_clock::now().time_since_epoch();
  uint64_t frameTime = std::chrono::duration_cast<std::chrono::milliseconds>(now).count();
  mouseDev.update(frameTime, *this);
  kbDev.update(frameTime, *this);
  xinputDev.update(frameTime, *this);
}

float Input::getGamepadDeadZone(int axis) {
  return xinputDev.deadZones[axis];
}

void Input::setGamepadDeadZone(int axis, float zoneRadius) {
  xinputDev.deadZones[axis] = zoneRadius;
}

LRESULT Input::procFn(HWND hwnd, WPARAM wparam, LPARAM lparam) {
  //delegate to default proc if window is not in foreground
  if(GET_RAWINPUT_CODE_WPARAM(wparam) != 0) { return DefWindowProc(hwnd, WM_INPUT, wparam, lparam); }

  //store the event to be handled during the update
  RAWINPUT rin;
  UINT size = sizeof(RAWINPUT);
  GetRawInputData(reinterpret_cast<HRAWINPUT>(lparam), RID_INPUT, &rin, &size, sizeof(RAWINPUTHEADER));

  switch(rin.header.dwType) {
  case RIM_TYPEMOUSE: mouseDev.enqueueEvent(rin); break;
  case RIM_TYPEKEYBOARD: kbDev.enqueueEvent(rin); break;
  }

  return 0;
}

//////////////////////////////////////////////////////////

Input::Device::Device(size_t buttonCt, size_t axisCt) {
  devState.buttons.resize(buttonCt);
  repeatData.resize(buttonCt);
  devState.axes.resize(axisCt);
}

void Input::Device::update(uint64_t frameTime, const Input& input) {
  for(auto& button : devState.buttons) { resetButton(button); }
  for(auto& axis : devState.axes) { axis = 0; }

  updateHandler(devState, eventQueue, frameTime);

  for(size_t i = 0; i < devState.buttons.size(); i++) { updateRepeat(i, frameTime, input); }
}

void Input::GamepadDevice::applyDeadZonedInput(DeviceState& devState, int axis, int input, int axisMaxRange) {
  devState.axes[axis] = static_cast<float>(input) / axisMaxRange;
  if(abs(devState.axes[axis]) < deadZones[axis]) { devState.axes[axis] = 0; }
}

void Input::Device::triggerButton(size_t index, uint64_t frameTime) {
  auto& btn = devState.buttons[index];
  auto& aux = repeatData[index];

  //for some reason RIN observes OS key repeat messages, so we need to ensure that the trigger is not spurious
  if(btn.held) { return; }

  btn.triggered = true;
  btn.held      = true;
  aux.triggerTimeMS = frameTime;
  aux.repeatPrev = 0;
}

void Input::Device::releaseButton(size_t index) {
  auto& btn = devState.buttons[index];
  btn.released = true;
  btn.held     = false;
}

void Input::Device::resetButton(DeviceButton& btn) {
  btn.triggered = false;
  btn.released  = false;
}

void Input::Device::updateRepeat(size_t index, uint64_t frameTime, const Input& input) {
  DeviceButton& btn = devState.buttons[index];
  ButtonRepeatData& aux = repeatData[index];

  //true on trigger frame
  btn.repeating = btn.triggered;
  if(btn.repeating) { return; }

  //if the key isn't down then it's not repeating (unless it triggered)
  if(!btn.held) { return; }

  //false prior to delay elapsed
  int elapsedMS = static_cast<int>(frameTime - aux.triggerTimeMS);
  int postDelayMS = elapsedMS - input.repeatDelayMS;
  if(postDelayMS < 0) { return; }

  //number of repeats that should have happened by now
  unsigned int repeatTarget = postDelayMS / input.repeatPeriodMS;
  //if it's increased since the last poll then repeat
  btn.repeating = aux.repeatPrev < repeatTarget;

  aux.repeatPrev = repeatTarget;
}

//////////////////////////////////////////////////////////

Input::KeyboardDevice::KeyboardDevice() : Device(BUTTON_CT, AXIS_CT) {
  // nop
}

Input::MouseDevice::MouseDevice() : Device(BUTTON_CT, AXIS_CT) {
  // nop
}

Input::GamepadDevice::GamepadDevice() : Device(BUTTON_CT, AXIS_CT) {
  buttonPrev.resize(BUTTON_CT);
  deadZones.resize(AXIS_CT, 0.1f);
}

void Input::KeyboardDevice::updateHandler(DeviceState& devState, std::queue<RAWINPUT>& eventQueue, uint64_t frameTime) {
  while(!eventQueue.empty()) {
    auto event = eventQueue.front().data.keyboard;

    if(event.Message == WM_KEYDOWN) { triggerButton(event.VKey, frameTime); }
    if(event.Message == WM_KEYUP)   { releaseButton(event.VKey); }

    eventQueue.pop();
  }
}

void Input::MouseDevice::updateHandler(DeviceState& devState, std::queue<RAWINPUT>& eventQueue, uint64_t frameTime) {
  while(!eventQueue.empty()) {
    auto event = eventQueue.front().data.mouse;

    devState.axes[Input::Mouse::DELTA_X] += event.lLastX;
    devState.axes[Input::Mouse::DELTA_Y] += event.lLastY;
    devState.axes[Input::Mouse::DELTA_WHEEL] += static_cast<short>(event.usButtonData);

    for(size_t i = 0; i < devState.buttons.size(); i++) {
      USHORT buttonState = event.usButtonFlags >> (i * 2);
      if(buttonState & 0b01) { triggerButton(i, frameTime); }
      if(buttonState & 0b10) { releaseButton(i); }
    }

    eventQueue.pop();
  }
}

void Input::GamepadDevice::updateHandler(DeviceState& devState, std::queue<RAWINPUT>& eventQueue, uint64_t frameTime) {
  if(!eventQueue.empty()) {
    throw std::runtime_error("XInput device recieved a Raw Input event message. Something is very wrong!");
  }

  XINPUT_STATE xstate;
  XInputGetState(0, &xstate);
  auto& pad = xstate.Gamepad;

  constexpr int xinBtnMap[] = {
    XINPUT_GAMEPAD_DPAD_UP, XINPUT_GAMEPAD_DPAD_DOWN, XINPUT_GAMEPAD_DPAD_LEFT, XINPUT_GAMEPAD_DPAD_RIGHT,
    XINPUT_GAMEPAD_START, XINPUT_GAMEPAD_BACK,
    XINPUT_GAMEPAD_LEFT_THUMB, XINPUT_GAMEPAD_RIGHT_THUMB,
    XINPUT_GAMEPAD_LEFT_SHOULDER, XINPUT_GAMEPAD_RIGHT_SHOULDER,
    XINPUT_GAMEPAD_A, XINPUT_GAMEPAD_B, XINPUT_GAMEPAD_X, XINPUT_GAMEPAD_Y
  };

  for(size_t i = 0; i < devState.buttons.size(); i++) {
    auto& btn = devState.buttons[i];

    btn.held = pad.wButtons & xinBtnMap[i];

    if( btn.held && !buttonPrev[i].held) { triggerButton(i, frameTime); }
    if(!btn.held &&  buttonPrev[i].held) { releaseButton(i); }
  }
  
  constexpr int XINPUT_STICK_RANGE = 32768;
  constexpr int XINPUT_TRIGGER_RANGE = 255;
  applyDeadZonedInput(devState, Gamepad::Axes::LEFT_X,   pad.sThumbLX, XINPUT_STICK_RANGE);
  applyDeadZonedInput(devState, Gamepad::Axes::LEFT_Y,   pad.sThumbLY, XINPUT_STICK_RANGE);
  applyDeadZonedInput(devState, Gamepad::Axes::RIGHT_X,  pad.sThumbRX, XINPUT_STICK_RANGE);
  applyDeadZonedInput(devState, Gamepad::Axes::RIGHT_Y,  pad.sThumbRY, XINPUT_STICK_RANGE);
  applyDeadZonedInput(devState, Gamepad::Axes::LTRIGGER, pad.bLeftTrigger,  XINPUT_TRIGGER_RANGE);
  applyDeadZonedInput(devState, Gamepad::Axes::RTRIGGER, pad.bRightTrigger, XINPUT_TRIGGER_RANGE);

  buttonPrev = devState.buttons;
}
