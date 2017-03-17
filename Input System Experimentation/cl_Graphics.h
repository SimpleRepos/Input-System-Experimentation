#pragma once
#include "st_ColorF.h"
#include <atlbase.h>
#include <d3d11.h>
#include "cl_Window.h"

class GfxFactory;

///<summary>Encapsulates core D3D interafaces</summary>
class Graphics {
public:
  ///<summary>Generate a graphics object attached to the provided window</summary>
  Graphics(Window& win);
  ~Graphics();

  ///<summary>Clear the buffers using the indicated color and default depth and stencil values - see Graphics::clearEx()</summary>
  void clear(const ColorF& color = ColorF::BLACK);

  ///<summary>Clear the indicated buffers using the indicated values</summary>
  void clearEx(bool clearTarget, bool clearDepth, bool clearStencil, const ColorF& color = ColorF::BLACK, float depth = 1.0f, UINT8 stencil = 0);

  ///<summary>Draw primitives using the assigned vertex buffer</summary>
  void draw(UINT vertexCount);

  ///<summary>Draw primitives using the assigned index and vertex buffers</summary>
  void drawIndexed(UINT indexCount, UINT startOffset = 0);

  ///<summary>Swap front and back buffers, wait for v-sync if enabled</summary>
  void present();

  Window::Dimensions VIEWPORT_DIMS;

  ///<summary>Generate a factory for creating objects that rely on the Graphics instance</summary>
  GfxFactory createFactory();

private:
  CComPtr<ID3D11Device> device;
  CComPtr<ID3D11DeviceContext> context;
  CComPtr<IDXGISwapChain> swapChain;
  CComPtr<ID3D11RenderTargetView> backBuffer;
  CComPtr<ID3D11DepthStencilView> depthStencilView;
  #if defined(DEBUG) || defined(_DEBUG)
  CComPtr<ID3D11Debug> debug;
  #endif

  void generateDeviceContextAndSwapChain(HWND winHandle);
  void generateBackBufferView();
  void generateDepthStencil();
  void setTargetsAndViewport();

};
