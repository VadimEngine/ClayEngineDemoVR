#include "Resource.h"

Resource::Resource() {}

Resource::~Resource() {}

template<typename T>
void Resource::addResource(std::unique_ptr<T> resource, const std::string& resourceName) {
    if constexpr (std::is_same_v<T, Model>) {
        mModels_[resourceName] = std::move(resource);
    } else if constexpr (std::is_same_v<T, Shader>) {
        mShaders_[resourceName] = std::move(resource);
    } else if constexpr (std::is_same_v<T, Audio>) {
        mAudios_[resourceName] = std::move(resource);
    } else if constexpr (std::is_same_v<T, Font>) {
        mFonts_[resourceName] = std::move(resource);
    } else if constexpr (std::is_same_v<T, Texture>) {
        mTextures_[resourceName] = std::move(resource);
    }
}

template<typename T>
T* Resource::getResource(const std::string& resourceName) {
    if constexpr (std::is_same_v<T, Model>) {
        auto it = mModels_.find(resourceName);
        if (it != mModels_.end()) {
            return it->second.get();
        }
    } else if constexpr (std::is_same_v<T, Shader>) {
        auto it = mShaders_.find(resourceName);
        if (it != mShaders_.end()) {
            return it->second.get();
        }
    } else if constexpr (std::is_same_v<T, Audio>) {
        auto it = mAudios_.find(resourceName);
        if (it != mAudios_.end()) {
            return it->second.get();
        }
    } else if constexpr (std::is_same_v<T, Font>) {
        auto it = mFonts_.find(resourceName);
        if (it != mFonts_.end()) {
            return it->second.get();
        }
    } else if constexpr (std::is_same_v<T, Texture>) {
        auto it = mTextures_.find(resourceName);
        if (it != mTextures_.end()) {
            return it->second.get();
        }
    }

    // Return nullptr if resource not found or type mismatch
    return nullptr;
}

// Explicit instantiate template for expected types
template void Resource::addResource(std::unique_ptr<Model> resource, const std::string& resourceName);
template void Resource::addResource(std::unique_ptr<Shader> resource, const std::string& resourceName);
template void Resource::addResource(std::unique_ptr<Audio> resource, const std::string& resourceName);
template void Resource::addResource(std::unique_ptr<Font> resource, const std::string& resourceName);
template void Resource::addResource(std::unique_ptr<Texture> resource, const std::string& resourceName);

template Model* Resource::getResource(const std::string& resourceName);
template Shader* Resource::getResource(const std::string& resourceName);
template Audio* Resource::getResource(const std::string& resourceName);
template Font* Resource::getResource(const std::string& resourceName);
template Texture* Resource::getResource(const std::string& resourceName);

