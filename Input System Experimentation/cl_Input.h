#pragma once
#include <unordered_map>
#include <array>
#include <chrono>
#include <queue>
#include "cl_Window.h"

//RIN https://msdn.microsoft.com/en-us/library/windows/desktop/ms645543(v=vs.85).aspx
//XIN https://msdn.microsoft.com/en-us/library/windows/desktop/ee417001(v=vs.85).aspx
//~~_ xinput with dead zones

class Input {
public:
  Input(Window& win);
  void update();

  struct MouseState;
  struct KeyboardState;

  const MouseState& mouse() const { return mouseState; }
  const KeyboardState& keyboard() const { return kbState; }

  //~~@ redo these to use non-member stuff and aux data so only the user-facing stuff is public

  struct MouseState {
    static const size_t BUTTON_CT = 5;
    struct Button { bool triggered, held, released; };
    Button buttons[BUTTON_CT] = {};

    int dX, dY, dWheel;

    void update();
    void addEvent(const RAWMOUSE& event);

  private:
    std::queue<RAWMOUSE> events;

  };

  struct KeyboardState {
    static const unsigned int REPEAT_DELAY_MS = 500;
    static const unsigned int REPEAT_FREQ_MS  = 100;
    static const size_t KEY_CT = 0xFF;

    struct Key {
      bool triggered, held, released, repeating;

      void trigger();
      void release();

    private:
      uint64_t triggerTimeMS;
      unsigned int repeatPrev;
      void updateRepeat();

    };

    Key keys[KEY_CT] = {};

    void update();
    void addEvent(const RAWKEYBOARD& event);

  private:
    std::queue<RAWKEYBOARD> events;

  };

private:
  MouseState mouseState;
  KeyboardState kbState;

  LRESULT procFn(HWND hwnd, WPARAM wparam, LPARAM lparam);

};
