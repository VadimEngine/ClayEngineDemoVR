#pragma once
// standard lib
#include <memory>
#include <string>
#include <unordered_map>
// third party
// project
#include "Core/Audio/Audio.h"
#include "Core/Graphics/Font.h"
#include "Core/Graphics/Model.h"
#include "Core/Graphics/Shader.h"
#include "Core/Graphics/Texture.h"
#include "Core/Graphics/Texture.h"

class Resource {
public:
    Resource();

    ~Resource();

    template<typename T>
    void addResource(std::unique_ptr<T> resource, const std::string& resourceName);

    template<typename T>
    T* getResource(const std::string& resourceName);

    std::unordered_map<std::string, std::unique_ptr<Audio>> mAudios_;
    std::unordered_map<std::string, std::unique_ptr<Model>> mModels_;
    std::unordered_map<std::string, std::unique_ptr<Shader>> mShaders_;
    std::unordered_map<std::string, std::unique_ptr<Font>> mFonts_;
    std::unordered_map<std::string, std::unique_ptr<Texture>> mTextures_;
};

