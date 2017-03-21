#include "cl_Input.h"

namespace {
  unsigned int millisecondsSinceEpoch() {
    auto now = std::chrono::high_resolution_clock::now();
    return std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch()).count();
  }
}

Input::Input(Window& win) {
  std::array<RAWINPUTDEVICE, 2> devices = {
    RAWINPUTDEVICE{ 1, 2, 0, win.getHandle() }, //mouse
    RAWINPUTDEVICE{ 1, 6, 0, win.getHandle() }  //kb
  };

  RegisterRawInputDevices(devices.data(), devices.size(), sizeof(RAWINPUTDEVICE));

  win.addProcFunc(WM_INPUT, [this](HWND hwnd, WPARAM wparam, LPARAM lparam) -> LRESULT { return procFn(hwnd, wparam, lparam); });
}

void Input::update() {
  mouseState.update();
  //keyboardState.update();
}

void Input::MouseState::update() {
  //clean the mouse state
  dX = 0;
  dY = 0;
  dWheel = 0;
  for(auto& button : buttons) {
    button.triggered = false;
    button.released  = false;
  }

  //process the event queue
  while(!events.empty()) {
    RAWMOUSE event = events.front();
    events.pop();

    dX += event.lLastX;
    dY += event.lLastY;
    short dWheelRaw = static_cast<short>(event.usButtonData);
    if(dWheelRaw < 0) { dWheel--; }
    else if(dWheelRaw > 0) { dWheel++; }
    for(int i = 0; i < MouseState::BUTTON_CT; i++) {
      USHORT buttonState = event.usButtonFlags >> (i * 2);
      if(buttonState & 0b01) {
        buttons[i].triggered = true;
        buttons[i].held      = true;
      }
      if(buttonState & 0b10) {
        buttons[i].released = true;
        buttons[i].held     = false;
      }
    }
  }
}

void Input::MouseState::addEvent(const RAWMOUSE& event) {
  events.push(event);
}

void Input::KeyboardState::update() {
  for(auto& key : keys) {
    key.triggered = false;
    key.released  = false;
  }

  while(!events.empty()) {
    RAWKEYBOARD event = events.front();
    events.pop();

    auto& key = keys[event.VKey];
    if(event.Message == WM_KEYDOWN)  { key.trigger(); }
    if(event.Message == WM_KEYUP)    { key.release(); }
  }
}

void Input::KeyboardState::addEvent(const RAWKEYBOARD& event) {
  events.push(event);
}

void Input::KeyboardState::Key::trigger() {
  triggered = true;
  held = true;
  triggerTimeMS = millisecondsSinceEpoch();
  repeatPrev = 0;
}

void Input::KeyboardState::Key::release() {
  released = true;
  held = false;
}

void Input::KeyboardState::Key::updateRepeat() {
  //true on trigger frame
  repeating = triggered;
  if(repeating) { return; }

  //if the key isn't down then it's not repeating (unless it triggered)
  if(!held) { return; }

  //false prior to delay elapsed
  int elapsedMS = millisecondsSinceEpoch() - triggerTimeMS;
  int postDelayMS = elapsedMS - REPEAT_DELAY_MS;
  if(postDelayMS < 0) { return; }

  //number of repeats that should have happened by now
  unsigned int repeatTarget = postDelayMS / REPEAT_FREQ_MS;
  //if it's increased since the last poll then repeat
  repeating = repeatPrev < repeatTarget;

  repeatPrev = repeatTarget;
}

LRESULT Input::procFn(HWND hwnd, WPARAM wparam, LPARAM lparam) {
  //delegate to default proc if window is not in foreground
  if(GET_RAWINPUT_CODE_WPARAM(wparam) != 0) { return DefWindowProc(hwnd, WM_INPUT, wparam, lparam); }

  //store the event to be handled during the update
  RAWINPUT rin;
  UINT size = sizeof(RAWINPUT);
  GetRawInputData(reinterpret_cast<HRAWINPUT>(lparam), RID_INPUT, &rin, &size, sizeof(RAWINPUTHEADER));

  switch(rin.header.dwType) {
  case    RIM_TYPEMOUSE: mouseState.addEvent(rin.data.mouse); break;
  case RIM_TYPEKEYBOARD: kbState.addEvent(rin.data.keyboard); break;
  }

  return 0;
}
