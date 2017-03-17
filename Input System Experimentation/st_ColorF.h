#pragma once
#include <cstdint>

///<summary>Struct for representing a color value (provides conversions)</summary>
struct ColorF {
  float r, g, b, a;

  //Predefined colors for convenience
  static const ColorF WHITE;
  static const ColorF BLACK;
  static const ColorF RED;
  static const ColorF YELLOW;
  static const ColorF GREEN;
  static const ColorF CYAN;
  static const ColorF BLUE;
  static const ColorF MAGENTA;
  static const ColorF ZERO;

  ///<summary>32-bit color union with per-channel control and uint32 access</summary>
  ///<remarks>Note that this obviously assumes a little endian system (such as desktop Windows)</remarks>
  union uCol32 {
    uint32_t uint;
    struct { uint8_t r, g, b, a; };
    uint8_t bytes[4];
  };

  //Conversion to float*
  operator float*() { return &r; }

  //Conversion to const float*
  operator const float*() const { return &r; }

  //conversion to uCol32
  operator uCol32() const {
    uCol32 val;
    val.r = static_cast<uint8_t>(r * 0xFF);
    val.g = static_cast<uint8_t>(g * 0xFF);
    val.b = static_cast<uint8_t>(b * 0xFF);
    val.a = static_cast<uint8_t>(a * 0xFF);
    return val;
  }

  //Conversion to 32-bit ABGR color
  operator uint32_t() const {
    uCol32 val = *this;
    return val.uint;
  }

};

