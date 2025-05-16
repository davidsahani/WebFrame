#include "utils.hpp"
#include "Log.hpp"

#include <fstream>
#define _CRT_SECURE_NO_WARNINGS
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

namespace Utils {

ID3D11ShaderResourceView *CreateDx11TextureRGBA(
    const void *data, int width, int height, ID3D11Device *d3dDevice
) {
    // Create texture
    D3D11_TEXTURE2D_DESC desc;
    ZeroMemory(&desc, sizeof(desc));
    desc.Width = width;
    desc.Height = height;
    desc.MipLevels = 1;
    desc.ArraySize = 1;
    desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    desc.SampleDesc.Count = 1;
    desc.Usage = D3D11_USAGE_DEFAULT;
    desc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
    desc.CPUAccessFlags = 0;

    ID3D11Texture2D *pTexture = nullptr;
    D3D11_SUBRESOURCE_DATA subResource;
    subResource.pSysMem = data;
    subResource.SysMemPitch = desc.Width * 4;
    subResource.SysMemSlicePitch = 0;
    if FAILED (d3dDevice->CreateTexture2D(&desc, &subResource, &pTexture)) {
        return nullptr;
    }

    // Create texture view
    D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc;
    ZeroMemory(&srvDesc, sizeof(srvDesc));
    srvDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
    srvDesc.Texture2D.MipLevels = desc.MipLevels;
    srvDesc.Texture2D.MostDetailedMip = 0;
    ID3D11ShaderResourceView *textureView = nullptr;
    if FAILED (d3dDevice->CreateShaderResourceView(pTexture, &srvDesc, &textureView)) {
        return nullptr;
    }

    pTexture->Release();
    return textureView;
}

ID3D11ShaderResourceView *CreateDx11TextureBGRA(
    const void *data, int width, int height, ID3D11Device *d3dDevice
) {
    // Create texture
    D3D11_TEXTURE2D_DESC desc;
    ZeroMemory(&desc, sizeof(desc));
    desc.Width = width;
    desc.Height = height;
    desc.MipLevels = 1;
    desc.ArraySize = 1;
    desc.Format = DXGI_FORMAT_B8G8R8A8_UNORM;
    desc.SampleDesc.Count = 1;
    desc.Usage = D3D11_USAGE_DEFAULT;
    desc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
    desc.CPUAccessFlags = 0;

    ID3D11Texture2D *pTexture = nullptr;
    D3D11_SUBRESOURCE_DATA subResource;
    subResource.pSysMem = data;
    subResource.SysMemPitch = desc.Width * 4;
    subResource.SysMemSlicePitch = 0;
    if FAILED (d3dDevice->CreateTexture2D(&desc, &subResource, &pTexture)) {
        return nullptr;
    }

    // Create texture view
    D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc;
    ZeroMemory(&srvDesc, sizeof(srvDesc));
    srvDesc.Format = DXGI_FORMAT_B8G8R8A8_UNORM;
    srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
    srvDesc.Texture2D.MipLevels = desc.MipLevels;
    srvDesc.Texture2D.MostDetailedMip = 0;
    ID3D11ShaderResourceView *textureView = nullptr;
    if FAILED (d3dDevice->CreateShaderResourceView(pTexture, &srvDesc, &textureView)) {
        return nullptr;
    }

    pTexture->Release();
    return textureView;
}

std::vector<char> ReadFileBuffer(const std::string &filepath) {
    std::ifstream file(filepath, std::ios::binary | std::ios::ate);
    if (!file) {
        Log::Error("Failed to open file: %s", filepath.c_str());
        return {};
    }

    size_t file_size = file.tellg();
    file.seekg(0, std::ios::beg);

    std::vector<char> file_data(file_size);
    if (!file.read(file_data.data(), file_size)) {
        Log::Error("Failed to read file: %s", filepath.c_str());
        return {};
    }
    file.close();
    return file_data;
}

// Open and read a file, then forward to LoadTextureFromMemory()
ID3D11ShaderResourceView *LoadTextureFromFile(const std::string &filepath, ID3D11Device *d3dDevice) {
    std::vector<char> file_buffer = ReadFileBuffer(filepath);

    if (file_buffer.empty()) return nullptr;

    // Load from disk into a raw RGBA buffer
    int image_width = 0;
    int image_height = 0;
    unsigned char *image_data = stbi_load_from_memory(
        (const unsigned char *)file_buffer.data(), (int)file_buffer.size(),
        &image_width, &image_height, NULL, 4
    );

    if (image_data == NULL) {
        Log::Error("Failed to load image: %s", filepath.c_str());
        return nullptr;
    };

    // Create texture
    ID3D11ShaderResourceView *textureView =
        CreateDx11TextureRGBA(image_data, image_width, image_height, d3dDevice);

    stbi_image_free(image_data);
    return textureView;
}

std::vector<ImTextureID> LoadImageTextures(std::vector<std::string> imagePaths, ID3D11Device *d3dDevice) {
    const ImTextureID placeholder_Texture = ImGui::GetIO().Fonts->TexID;
    std::vector<ImTextureID> textures;

    for (auto it = imagePaths.begin(); it != imagePaths.end(); ++it) {
        auto textureView = LoadTextureFromFile(*it, d3dDevice);

        if (textureView != nullptr) {
            textures.push_back((ImTextureID)(intptr_t)textureView);
        } else {
            textures.push_back(placeholder_Texture);
        }
    }

    return textures;
}

/**
 * @brief Captures the primary screen and returns a 32-bit BGRA buffer.
 *
 * Uses GDI to copy screen content into a bitmap, then retrieves BGRA pixel data.
 *
 * @param[out] outBGRAImage Raw pixel buffer in BGRA order.
 * @param[out] width Image width.
 * @param[out] height Image height.
 * @return true on success, false on error.
 */
bool CaptureScreenshot(std::vector<BYTE> &outBGRAImageBuffer, int &width, int &height) {
    // Get screen size
    width = GetSystemMetrics(SM_CXSCREEN);
    height = GetSystemMetrics(SM_CYSCREEN);

    // Get device context
    HDC hScreenDC = GetDC(NULL);
    if (!hScreenDC) return false;

    HDC hMemoryDC = CreateCompatibleDC(hScreenDC);
    if (!hMemoryDC) {
        ReleaseDC(NULL, hScreenDC);
        return false;
    }

    // Create compatible bitmap
    HBITMAP hBitmap = CreateCompatibleBitmap(hScreenDC, width, height);
    if (!hBitmap) {
        DeleteDC(hMemoryDC);
        ReleaseDC(NULL, hScreenDC);
        return false;
    }

    SelectObject(hMemoryDC, hBitmap);

    // Copy screen to bitmap
    if (!BitBlt(hMemoryDC, 0, 0, width, height, hScreenDC, 0, 0, SRCCOPY)) {
        DeleteObject(hBitmap);
        DeleteDC(hMemoryDC);
        ReleaseDC(NULL, hScreenDC);
        return false;
    }

    // Prepare bitmap info
    BITMAPINFOHEADER bi = {};
    bi.biSize = sizeof(BITMAPINFOHEADER);
    bi.biWidth = width;
    bi.biHeight = -height; // Negative to correct upside-down image
    bi.biPlanes = 1;
    bi.biBitCount = 32;
    bi.biCompression = BI_RGB;

    // Allocate buffer
    size_t imageSize = width * height * 4;
    outBGRAImageBuffer.resize(imageSize);
    if (!GetDIBits(hMemoryDC, hBitmap, 0, height, outBGRAImageBuffer.data(), (BITMAPINFO *)&bi, DIB_RGB_COLORS)) {
        DeleteObject(hBitmap);
        DeleteDC(hMemoryDC);
        ReleaseDC(NULL, hScreenDC);
        return false;
    }

    // Cleanup
    DeleteObject(hBitmap);
    DeleteDC(hMemoryDC);
    ReleaseDC(NULL, hScreenDC);

    return true;
}

/**
 * @brief Copies a top-down BGRA image to the clipboard as a 24-bit CF_BITMAP.
 *
 * @param BGRAImage       Top-down BGRA image data (width * height * 4).
 * @param width           Image width in pixels.
 * @param height          Image height in pixels.
 * @return true           If successfully placed on the clipboard, false otherwise.
 *
 * @note Paint often expects a 24-bit device-dependent bitmap (CF_BITMAP).
 *       This function discards the alpha channel.
 */
bool Utils::CopyImageToClipboard(const std::vector<BYTE> &BGRAImage, int width, int height) {
    if (BGRAImage.size() < static_cast<size_t>(width * height * 4))
        return false;

    // Create a compatible DC/bitmap for the screen
    HDC hScreenDC = GetDC(nullptr);
    if (!hScreenDC)
        return false;

    HDC hMemoryDC = CreateCompatibleDC(hScreenDC);
    if (!hMemoryDC) {
        ReleaseDC(nullptr, hScreenDC);
        return false;
    }

    // Create a device-dependent bitmap (DDB)
    HBITMAP hBitmap = CreateCompatibleBitmap(hScreenDC, width, height);
    if (!hBitmap) {
        DeleteDC(hMemoryDC);
        ReleaseDC(nullptr, hScreenDC);
        return false;
    }

    // Select it into the memory DC
    HBITMAP hOldBmp = (HBITMAP)SelectObject(hMemoryDC, hBitmap);

    // Prepare a BITMAPINFO for 24-bit data
    BITMAPINFO bi = {};
    bi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
    bi.bmiHeader.biWidth = width;
    bi.bmiHeader.biHeight = height; // positive -> bottom-up
    bi.bmiHeader.biPlanes = 1;
    bi.bmiHeader.biBitCount = 24;
    bi.bmiHeader.biCompression = BI_RGB;

    // Convert top-down BGRA → bottom-up BGR (24-bit, discarding alpha)
    std::vector<BYTE> bgr24(width * height * 3);
    const int rowSizeSrc = width * 4; // BGRA
    const int rowSizeDst = width * 3; // BGR
    for (int y = 0; y < height; ++y) {
        // Source row (top-down)
        const BYTE *srcLine = BGRAImage.data() + (y * rowSizeSrc);
        // Destination row (bottom-up)
        BYTE *dstLine = bgr24.data() + ((height - 1 - y) * rowSizeDst);
        for (int x = 0; x < width; ++x) {
            // B, G, R from BGRA (ignore alpha)
            dstLine[x * 3 + 0] = srcLine[x * 4 + 0]; // B
            dstLine[x * 3 + 1] = srcLine[x * 4 + 1]; // G
            dstLine[x * 3 + 2] = srcLine[x * 4 + 2]; // R
        }
    }

    // Transfer the 24-bit data into the device-dependent bitmap
    if (SetDIBits(hMemoryDC, hBitmap, 0, height, bgr24.data(), &bi, DIB_RGB_COLORS) == 0) {
        // If SetDIBits fails, cleanup
        SelectObject(hMemoryDC, hOldBmp);
        DeleteObject(hBitmap);
        DeleteDC(hMemoryDC);
        ReleaseDC(nullptr, hScreenDC);
        return false;
    }

    // Restore old bitmap in memory DC
    SelectObject(hMemoryDC, hOldBmp);

    // Place the HBITMAP on the clipboard
    if (!OpenClipboard(nullptr)) {
        DeleteObject(hBitmap);
        DeleteDC(hMemoryDC);
        ReleaseDC(nullptr, hScreenDC);
        return false;
    }
    EmptyClipboard();
    if (!SetClipboardData(CF_BITMAP, hBitmap)) {
        CloseClipboard();
        DeleteObject(hBitmap);
        DeleteDC(hMemoryDC);
        ReleaseDC(nullptr, hScreenDC);
        return false;
    }
    CloseClipboard();

    // hBitmap is now owned by the clipboard; do not delete it
    DeleteDC(hMemoryDC);
    ReleaseDC(nullptr, hScreenDC);

    return true;
}

} // namespace Utils
