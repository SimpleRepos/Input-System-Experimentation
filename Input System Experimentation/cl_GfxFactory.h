#pragma once
#include <string>
#include <vector>
#include <atlbase.h>
#include <d3d11.h>
#include "cl_Font.h"
#include <memory>
#include <DirectXMath.h>

///<summary>Factory for graphics objects</summary>
class GfxFactory {
public:
  GfxFactory(ID3D11Device* device, ID3D11DeviceContext* context);
  GfxFactory(GfxFactory&&) = default;
  ~GfxFactory();

  ///<summary>Generate a Font object</summary>
  Font createFont(const std::wstring& fontFace);

private:
  CComPtr<IFW1Factory> fontFactory;
  CComPtr<ID3D11Device> device;
  CComPtr<ID3D11DeviceContext> context;

};

