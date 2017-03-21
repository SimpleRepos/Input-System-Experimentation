#pragma once
#include <unordered_map>
#include <array>
#include <chrono>
#include "cl_Window.h"

//RIN https://msdn.microsoft.com/en-us/library/windows/desktop/ms645543(v=vs.85).aspx
//XIN https://msdn.microsoft.com/en-us/library/windows/desktop/ee417001(v=vs.85).aspx
//~~_ repeat
//~~_ dead zones

class Input {
public:
  Input(Window& win);
  void update();

  struct MouseState {
    struct Button {
      bool triggered, held, released;
    };

    static const size_t BUTTON_CT = 5;
    Button buttons[BUTTON_CT];
    int dX, dY, dWheel;
  };

  const MouseState& mouse() const { return mouseState; }

  static const unsigned int REPEAT_DELAY_MS = 500;
  static const unsigned int REPEAT_FREQ_MS = 100;

  struct KeyboardState {
    struct Key {
      bool triggered, held, released, repeating;

      void trigger() {
        triggered = true;
        held = true;
        triggerTimeMS = millisecondsSinceEpoch();
      }

      void release() {
        released = true;
        held = false;
      }

      unsigned int millisecondsSinceEpoch() const {
        auto now = std::chrono::high_resolution_clock::now();
        return std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch()).count();
      }

      void calcRepeat(unsigned int msSinceEpoch) {
        //true on trigger frame
        repeating = triggered;
        if(repeating) { return; }

        //false prior to delay elapsed
        int elapsedMS = msSinceEpoch - triggerTimeMS;
        int postDelayMS = elapsedMS - REPEAT_DELAY_MS;
        if(postDelayMS < 0) { return; }

        //number of repeats that should have happened by now
        unsigned int repeatTarget = postDelayMS / REPEAT_FREQ_MS;
        //if it's increased since the last poll then repeat
        repeating = repeatPrev < repeatTarget;

        repeatPrev = repeatTarget;
      }

    private:
      unsigned int triggerTimeMS;
      unsigned int repeatPrev;

    };

    static const size_t KEY_CT = 200;
    Key keys[KEY_CT];
  };

  const KeyboardState& keyboard() const { return kbState; }

private:
  MouseState mouseState;
  KeyboardState kbState;

  LRESULT procFn(HWND hwnd, WPARAM wparam, LPARAM lparam);

};
