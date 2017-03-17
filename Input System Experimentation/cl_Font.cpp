#include "cl_Font.h"

#pragma comment(lib, "FW1FontWrapper_1_1/FW1FontWrapper.lib")

void Font::drawText(const std::wstring& text, float size, float x, float y, ColorF color) {
  wrapper->DrawString(context, text.c_str(), size, x, y, color, FW1_RESTORESTATE);
}

Font::Font(IFW1Factory* factory, ID3D11Device* device, ID3D11DeviceContext* context, const std::wstring& fontFace) :
  context(context)
{
  factory->CreateFontWrapper(device, fontFace.c_str(), &wrapper);
}
