#include "cl_Input.h"
#include <Xinput.h>

#pragma comment(lib, "Xinput9_1_0.lib")

Input::Device::Device(size_t buttonCt, size_t axisCt, Type type) : type(type) {
  repeatDelayMS = DEFAULT_REPEAT_DELAY_MS;
  repeatPeriodMS = DEFAULT_REPEAT_PERIOD_MS;

  devState.buttons.resize(buttonCt);
  repeatData.resize(buttonCt);
  devState.axes.resize(axisCt);

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
    XINPUT_STATE xstate;
    XInputGetState(0, &xstate);
    auto& pad = xstate.Gamepad;

    for(size_t i = 0; i < devState.buttons.size(); i++) {
      auto& btn = devState.buttons[i];
      btn.held = (pad.wButtons >> i) & 1;
      if(btn.held && !xinputPrev[i].held) { triggerButton(btn, repeatData[i], frameTime); }
      if(!btn.held && xinputPrev[i].held) { releaseButton(btn); }
    }

    devState.axes[Gamepad::LEFT_X] = pad.sThumbLX;
    devState.axes[Gamepad::LEFT_Y] = pad.sThumbLY;
    devState.axes[Gamepad::RIGHT_X] = pad.sThumbRX;
    devState.axes[Gamepad::RIGHT_Y] = pad.sThumbRY;
    devState.axes[Gamepad::LTRIGGER] = pad.bLeftTrigger;
    devState.axes[Gamepad::RTRIGGER] = pad.bRightTrigger;

    xinputPrev = devState.buttons;
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
  kbDev(0xFF, 0, Device::Type::KEYBOARD),
  mouseDev(5, 3, Device::Type::MOUSE),
  xinputDev(16, 6, Device::Type::XINPUT)
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
