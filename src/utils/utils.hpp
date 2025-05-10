#pragma once
#include <string>
#include <vector>
#include <unordered_map>
#include <d3d11.h>
#include "imgui.h"

class ImageTextures {
  public:
    ImTextureID Get(const std::string &name) const {
        if (textures.find(name) != textures.end()) {
            return textures.at(name);
        }
        return ImGui::GetIO().Fonts->TexID; // default texture
    };

    ImageTextures(std::unordered_map<std::string, ImTextureID> textures) : textures(textures) {}

  private:
    std::unordered_map<std::string, ImTextureID> textures;
};

namespace Utils {

ImageTextures LoadImageTextures(std::vector<std::pair<std::string, std::string>> imagePaths, ID3D11Device *d3dDevice);
bool CaptureScreenshot(std::vector<BYTE> &outBGRAImageBuffer, int &width, int &height);
bool CopyImageToClipboard(const std::vector<BYTE> &BGRAImageBuffer, int width, int height);
ID3D11ShaderResourceView *CreateDx11TextureBGRA(const void *data, int width, int height, ID3D11Device *d3dDevice);

} // namespace Utils
