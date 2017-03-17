#include "cl_Graphics.h"
#include <cassert>
#include "ns_Utility.h"
#include "cl_GfxFactory.h"

#pragma comment(lib, "d3d11.lib")

Graphics::Graphics(Window& win) :
  VIEWPORT_DIMS(win.USER_AREA_DIMS)
{
  generateDeviceContextAndSwapChain(win.getHandle());
  generateBackBufferView();
  generateDepthStencil();
  setTargetsAndViewport();
}

Graphics::~Graphics() {
  #if defined(DEBUG) || defined(_DEBUG)
  //debug->ReportLiveDeviceObjects(D3D11_RLDO_DETAIL);
  #endif
}

void Graphics::clear(const ColorF& color) {
  clearEx(true, true, true, color, 1.0f, 0);
}

void Graphics::clearEx(bool clearTarget, bool clearDepth, bool clearStencil, const ColorF& color, float depth, UINT8 stencil) {
  if(clearTarget) { context->ClearRenderTargetView(backBuffer, color); }

  UINT flags = 0;
  if(clearDepth)   { flags |= D3D11_CLEAR_DEPTH; }
  if(clearStencil) { flags |= D3D11_CLEAR_STENCIL; }
  if(flags) { context->ClearDepthStencilView(depthStencilView, flags, depth, stencil); }
}

void Graphics::draw(UINT vertexCount) {
  context->Draw(vertexCount, 0);
}

void Graphics::drawIndexed(UINT indexCount, UINT startOffset) {
  context->DrawIndexed(indexCount, 0, startOffset);
}

void Graphics::present() {
  swapChain->Present(0, 0);
}

GfxFactory Graphics::createFactory() {
  return GfxFactory(device.p, context.p);
}

void Graphics::generateDeviceContextAndSwapChain(HWND winHandle) {
  DXGI_SWAP_CHAIN_DESC sd = {
    DXGI_MODE_DESC{
      (UINT)VIEWPORT_DIMS.width, (UINT)VIEWPORT_DIMS.height,
      DXGI_RATIONAL{ 60, 1 },
      DXGI_FORMAT_R8G8B8A8_UNORM,
      DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED,
      DXGI_MODE_SCALING_UNSPECIFIED
    },
    DXGI_SAMPLE_DESC{ 1, 0 },
    DXGI_USAGE_RENDER_TARGET_OUTPUT,
    1,
    winHandle,
    true,
    DXGI_SWAP_EFFECT_DISCARD,
    0
  };

  UINT flags = D3D11_CREATE_DEVICE_SINGLETHREADED;
  #if defined(DEBUG) || defined(_DEBUG)
  flags |= D3D11_CREATE_DEVICE_DEBUG;
  #endif

  HR(D3D11CreateDeviceAndSwapChain(0, D3D_DRIVER_TYPE_HARDWARE, 0, flags, 0, 0, D3D11_SDK_VERSION, &sd, &swapChain, &device, 0, &context));

  #if defined(DEBUG) || defined(_DEBUG)
  device->QueryInterface(IID_PPV_ARGS(&debug));
  #endif
}

void Graphics::generateBackBufferView() {
  CComPtr<ID3D11Texture2D> bbTex;
  HR(swapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (void**)&bbTex.p));
  HR(device->CreateRenderTargetView(bbTex, 0, &backBuffer));
}

void Graphics::generateDepthStencil() {
  D3D11_TEXTURE2D_DESC dbDesc = {
    (UINT)VIEWPORT_DIMS.width, (UINT)VIEWPORT_DIMS.height,
    1, 1,
    DXGI_FORMAT_D24_UNORM_S8_UINT,
    DXGI_SAMPLE_DESC{ 1, 0 },
    D3D11_USAGE_DEFAULT,
    D3D11_BIND_DEPTH_STENCIL,
    0, 0
  };

  CComPtr<ID3D11Texture2D> depthStencilBuffer;
  HR(device->CreateTexture2D(&dbDesc, 0, &depthStencilBuffer));
  HR(device->CreateDepthStencilView(depthStencilBuffer, 0, &depthStencilView));
}

void Graphics::setTargetsAndViewport() {
  context->OMSetRenderTargets(1, &backBuffer.p, depthStencilView);
  
  D3D11_VIEWPORT vp = { 0.0f, 0.0f, (float)VIEWPORT_DIMS.width, (float)VIEWPORT_DIMS.height, 0.0f, 1.0f };
  context->RSSetViewports(1, &vp);
}
