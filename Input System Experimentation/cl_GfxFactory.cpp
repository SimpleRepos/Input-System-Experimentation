#include "cl_GfxFactory.h"
#include "FW1FontWrapper_1_1/FW1FontWrapper.h"
#include "ns_Utility.h"

GfxFactory::GfxFactory(ID3D11Device* device, ID3D11DeviceContext* context) : device(device), context(context) {
  HR(FW1CreateFactory(FW1_VERSION, &fontFactory));
}

GfxFactory::~GfxFactory() {}

Font GfxFactory::createFont(const std::wstring& fontFace) {
  return Font(fontFactory, device, context, fontFace);
}
