#include "cl_Input.h"

Input::Input(Window& win) {
  repeatDelayMS = DEFAULT_REPEAT_DELAY_MS;
  repeatPeriodMS = DEFAULT_REPEAT_PERIOD_MS;

  std::array<RAWINPUTDEVICE, 2> devices = {
    RAWINPUTDEVICE{ 1, 2, 0, win.getHandle() }, //mouse
    RAWINPUTDEVICE{ 1, 6, 0, win.getHandle() }  //kb
  };

  RegisterRawInputDevices(devices.data(), devices.size(), sizeof(RAWINPUTDEVICE));

  win.addProcFunc(WM_INPUT, [this](HWND hwnd, WPARAM wparam, LPARAM lparam) -> LRESULT { return procFn(hwnd, wparam, lparam); });
}

void Input::update() {
  auto now = std::chrono::high_resolution_clock::now().time_since_epoch();
  uint64_t frameTime = std::chrono::duration_cast<std::chrono::milliseconds>(now).count();
  updateMouseState(frameTime);
  updateKeyboardState(frameTime);
}

void Input::triggerButton(Button& btn, ButtonRepeatData& aux, uint64_t frameTime) {
  btn.triggered = true;
  btn.held      = true;
  aux.triggerTimeMS = frameTime;
  aux.repeatPrev = 0;
}

void Input::releaseButton(Button& btn) {
  btn.released = true;
  btn.held     = false;
}

void Input::updateMouseState(uint64_t frameTime) {
  for(auto& button : mouseState.buttons) { resetButton(button); }
  for(auto& axis : mouseState.axes) { axis = 0; }

  while(!mouseEvents.empty()) {
    RAWMOUSE event = mouseEvents.front();
    mouseEvents.pop();
    processMouseEvent(event, frameTime);
  }

  for(int i = 0; i < MouseState::BUTTON_CT; i++) { updateRepeat(mouseState.buttons[i], mouseAux[i], frameTime); }
}

void Input::processMouseEvent(const RAWMOUSE& event, uint64_t frameTime) {
  mouseState.axes[MouseState::DELTA_X] += event.lLastX;
  mouseState.axes[MouseState::DELTA_Y] += event.lLastY;
  mouseState.axes[MouseState::DELTA_WHEEL] += static_cast<short>(event.usButtonData);

  for(int i = 0; i < MouseState::BUTTON_CT; i++) {
    USHORT buttonState = event.usButtonFlags >> (i * 2);
    if(buttonState & 0b01) { triggerButton(mouseState.buttons[i], mouseAux[i], frameTime); }
    if(buttonState & 0b10) { releaseButton(mouseState.buttons[i]); }
  }
}

void Input::updateKeyboardState(uint64_t frameTime) {
  for(auto& key : kbState.buttons) { resetButton(key); }

  while(!kbEvents.empty()) {
    RAWKEYBOARD event = kbEvents.front();
    kbEvents.pop();
    processKeyboardEvent(event, frameTime);
  }

  for(int i = 0; i < KeyboardState::BUTTON_CT; i++) { updateRepeat(kbState.buttons[i], kbAux[i], frameTime); }
}

void Input::processKeyboardEvent(const RAWKEYBOARD& event, uint64_t frameTime) {
  if(event.Message == WM_KEYDOWN) { triggerButton(kbState.buttons[event.VKey], kbAux[event.VKey], frameTime); }
  if(event.Message == WM_KEYUP)   { releaseButton(kbState.buttons[event.VKey]); }
}

void Input::resetButton(Button& btn) {
  btn.triggered = false;
  btn.released  = false;
}

void Input::updateRepeat(Button& btn, ButtonRepeatData& aux, uint64_t frameTime) {
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

LRESULT Input::procFn(HWND hwnd, WPARAM wparam, LPARAM lparam) {
  //delegate to default proc if window is not in foreground
  if(GET_RAWINPUT_CODE_WPARAM(wparam) != 0) { return DefWindowProc(hwnd, WM_INPUT, wparam, lparam); }

  //store the event to be handled during the update
  RAWINPUT rin;
  UINT size = sizeof(RAWINPUT);
  GetRawInputData(reinterpret_cast<HRAWINPUT>(lparam), RID_INPUT, &rin, &size, sizeof(RAWINPUTHEADER));

  switch(rin.header.dwType) {
  case    RIM_TYPEMOUSE: mouseEvents.push(rin.data.mouse); break;
  case RIM_TYPEKEYBOARD: kbEvents.push(rin.data.keyboard); break;
  }

  return 0;
}
