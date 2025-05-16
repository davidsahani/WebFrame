#pragma once
#include <string>
#include <vector>
#include <d3d11.h>
#include "imgui.h"

namespace Utils {

bool CaptureScreenshot(std::vector<BYTE> &outBGRAImageBuffer, int &width, int &height);
bool CopyImageToClipboard(const std::vector<BYTE> &BGRAImageBuffer, int width, int height);
ID3D11ShaderResourceView *CreateDx11TextureBGRA(const void *data, int width, int height, ID3D11Device *d3dDevice);
std::vector<ImTextureID> LoadImageTextures(std::vector<std::string> imagePaths, ID3D11Device *d3dDevice);

} // namespace Utils
