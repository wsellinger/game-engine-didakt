#pragma once

#include "providers/interfaces/IAssetProvider.h"
#include "render/texturehandle.h"

#include <string>

class MockAssetProvider : public IAssetProvider
{
public:
    TextureHandle LoadTexture(const std::string& path) override
    {
        loadCallCount++;
        lastLoadedPath = path;

        if (shouldFailLoad)
            return TextureHandle{};

        // Fabricate a distinct non-null handle per call so tests can
        // tell different loaded textures apart.
        static int nextHandleId = 1;
        void* fakeHandle = reinterpret_cast<void*>(static_cast<intptr_t>(nextHandleId++));
        return TextureHandle{ fakeHandle };
    }

    void DestroyTexture(TextureHandle texture) override
    {
        destroyCallCount++;
        lastDestroyedHandle = texture;
    }

    // Test-controlled behavior
    bool shouldFailLoad = false;

    // Test-observable state
    int loadCallCount = 0;
    int destroyCallCount = 0;
    std::string lastLoadedPath;
    TextureHandle lastDestroyedHandle{};
};