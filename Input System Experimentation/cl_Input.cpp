#include "cl_Input.h"
#include <Xinput.h>

#pragma comment(lib, "Xinput9_1_0.lib")

Input::Device::Device(size_t buttonCt, size_t axisCt, Type type) : type(type) {
  repeatDelayMS = DEFAULT_REPEAT_DELAY_MS;
  repeatPeriodMS = DEFAULT_REPEAT_PERIOD_MS;

  devState.buttons.resize(buttonCt);
  repeatData.resize(buttonCt);
  devState.axes.resize(axisCt);
  deadZones.resize(axisCt, 0.1f);

  switch(type) {
  case Type::MOUSE:
    processEvent = [this](const RAWINPUT& rin, uint64_t frameTime) { mouseHandler(rin, frameTime); };
    break;
  case Type::KEYBOARD:
    processEvent = [this](const RAWINPUT& rin, uint64_t frameTime) { keyboardHandler(rin, frameTime); };
    break;
  case Type::XINPUT:
    processEvent = [](const RAWINPUT&, uint64_t) {};
    xinputPrev = devState.buttons;
    break;
  }
}

void Input::Device::update(uint64_t frameTime) {
  for(auto& button : devState.buttons) { resetButton(button); }
  for(auto& axis : devState.axes) { axis = 0; }

  if(type == Type::XINPUT) {
    updateXinput(frameTime);
  }
  else {
    while(!eventQueue.empty()) {
      auto event = eventQueue.front();
      eventQueue.pop();
      processEvent(event, frameTime);
    }
  }

  for(size_t i = 0; i < devState.buttons.size(); i++) {
    updateRepeat(devState.buttons[i], repeatData[i], frameTime);
  }
}

void Input::Device::updateXinput(uint64_t frameTime) {
  XINPUT_STATE xstate;
  XInputGetState(0, &xstate);
  auto& pad = xstate.Gamepad;

  for(size_t i = 0; i < devState.buttons.size(); i++) {
    size_t effectiveIndex = (i < Gamepad::Buttons::A) ? i : i + 2;
    auto& btn = devState.buttons[i];
    btn.held = (pad.wButtons >> effectiveIndex) & 1;
    if(btn.held && !xinputPrev[i].held) { triggerButton(btn, repeatData[i], frameTime); }
    if(!btn.held && xinputPrev[i].held) { releaseButton(btn); }
  }

  constexpr float STICK_RANGE = 32768.0f;
  constexpr float TRIGGER_RANGE = 255.0f;

  applyDeadZonedInput(Gamepad::Axes::LEFT_X,  pad.sThumbLX, STICK_RANGE);
  applyDeadZonedInput(Gamepad::Axes::LEFT_Y,  pad.sThumbLY, STICK_RANGE);
  applyDeadZonedInput(Gamepad::Axes::RIGHT_X, pad.sThumbRX, STICK_RANGE);
  applyDeadZonedInput(Gamepad::Axes::RIGHT_Y, pad.sThumbRY, STICK_RANGE);
  applyDeadZonedInput(Gamepad::Axes::LTRIGGER, pad.bLeftTrigger,  TRIGGER_RANGE);
  applyDeadZonedInput(Gamepad::Axes::RTRIGGER, pad.bRightTrigger, TRIGGER_RANGE);

  xinputPrev = devState.buttons;
}

void Input::Device::applyDeadZonedInput(int axis, int input, float axisMaxRange) {
  devState.axes[axis] = static_cast<float>(input) / axisMaxRange;
  if(abs(devState.axes[axis]) < deadZones[axis]) { devState.axes[axis] = 0; }
}

void Input::Device::triggerButton(Button& btn, ButtonRepeatData& aux, uint64_t frameTime) {
  btn.triggered = true;
  btn.held      = true;
  aux.triggerTimeMS = frameTime;
  aux.repeatPrev = 0;
}

void Input::Device::releaseButton(Button& btn) {
  btn.released = true;
  btn.held     = false;
}

void Input::Device::resetButton(Button& btn) {
  btn.triggered = false;
  btn.released  = false;
}

void Input::Device::updateRepeat(Button& btn, ButtonRepeatData& aux, uint64_t frameTime) {
  //true on trigger frame
  btn.repeating = btn.triggered;
  if(btn.repeating) { return; }

  //if the key isn't down then it's not repeating (unless it triggered)
  if(!btn.held) { return; }

  //false prior to delay elapsed
  int elapsedMS = static_cast<int>(frameTime - aux.triggerTimeMS);
  int postDelayMS = elapsedMS - repeatDelayMS;
  if(postDelayMS < 0) { return; }

  //number of repeats that should have happened by now
  unsigned int repeatTarget = postDelayMS / repeatPeriodMS;
  //if it's increased since the last poll then repeat
  btn.repeating = aux.repeatPrev < repeatTarget;

  aux.repeatPrev = repeatTarget;
}

void Input::Device::keyboardHandler(const RAWINPUT& rin, uint64_t frameTime) {
  auto& event = rin.data.keyboard;
  if(event.Message == WM_KEYDOWN) { triggerButton(devState.buttons[event.VKey], repeatData[event.VKey], frameTime); }
  if(event.Message == WM_KEYUP)   { releaseButton(devState.buttons[event.VKey]); }
}

void Input::Device::mouseHandler(const RAWINPUT& rin, uint64_t frameTime) {
  auto& event = rin.data.mouse;
  devState.axes[Input::Mouse::DELTA_X] += event.lLastX;
  devState.axes[Input::Mouse::DELTA_Y] += event.lLastY;
  devState.axes[Input::Mouse::DELTA_WHEEL] += static_cast<short>(event.usButtonData);
  
  for(size_t i = 0; i < devState.buttons.size(); i++) {
    USHORT buttonState = event.usButtonFlags >> (i * 2);
    if(buttonState & 0b01) { triggerButton(devState.buttons[i], repeatData[i], frameTime); }
    if(buttonState & 0b10) { releaseButton(devState.buttons[i]); }
  }
}

//////////////////////////////////////////

Input::Input(Window& win) :
  kbDev(KB_BUTTON_CT, KB_AXIS_CT, Device::Type::KEYBOARD),
  mouseDev(MOUSE_BUTTON_CT, MOUSE_AXIS_CT, Device::Type::MOUSE),
  xinputDev(XIN_BUTTON_CT, XIN_AXIS_CT, Device::Type::XINPUT)
{
  constexpr size_t NUM_DEVICES = 2;
  RAWINPUTDEVICE devices[NUM_DEVICES] = {
    RAWINPUTDEVICE{ 1, 2, 0, win.getHandle() }, //mouse
    RAWINPUTDEVICE{ 1, 6, 0, win.getHandle() }  //kb
  };

  RegisterRawInputDevices(devices, NUM_DEVICES, sizeof(RAWINPUTDEVICE));

  win.addProcFunc(WM_INPUT, [this](HWND hwnd, WPARAM wparam, LPARAM lparam) -> LRESULT { return procFn(hwnd, wparam, lparam); });
}

void Input::update() {
  auto now = std::chrono::high_resolution_clock::now().time_since_epoch();
  uint64_t frameTime = std::chrono::duration_cast<std::chrono::milliseconds>(now).count();
  mouseDev.update(frameTime);
  kbDev.update(frameTime);
  xinputDev.update(frameTime);
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
  case RIM_TYPEMOUSE: mouseDev.eventQueue.push(rin); break;
  case RIM_TYPEKEYBOARD: kbDev.eventQueue.push(rin); break;
  }

  return 0;
}
