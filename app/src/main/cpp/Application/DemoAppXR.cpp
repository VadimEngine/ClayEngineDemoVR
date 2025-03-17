// project
#include "Application/Scenes/Farm/FarmScene.h"
#include "Application/Scenes/Sandbox/SandboxScene.h"
#include "Application/Scenes/Space/SpaceScene.h"
// class
#include "Application/DemoAppXR.h"

DemoAppXR::DemoAppXR(android_app *pAndroidApp) :
    clay::AppXR(pAndroidApp) {}

void DemoAppXR::createResources() {
    // Shaders
    mResources_.loadResource<clay::ShaderProgram>(
        {
            "Shaders/SimpleVert.vert:VERTEX",
            "Shaders/SimpleFrag.frag:FRAGMENT"
        },
        "SimpleShader"
    );
    mResources_.loadResource<clay::ShaderProgram>(
        {
            "Shaders/FlatVert.vert:VERTEX",
            "Shaders/FlatFrag.frag:FRAGMENT"
        },
        "FlatShader"
    );
    mResources_.loadResource<clay::ShaderProgram>(
        {
            "Shaders/TextVert.vert:VERTEX",
            "Shaders/TextFrag.frag:FRAGMENT"
        },
        "TextShader"
    );
    mResources_.loadResource<clay::ShaderProgram>(
        {
            "Shaders/TextureVert.vert:VERTEX",
            "Shaders/TextureFrag.frag:FRAGMENT"
        },
        "TextureShader"
    );
    mResources_.loadResource<clay::ShaderProgram>(
        {
            "Shaders/TextureFlipVert.vert:VERTEX",
            "Shaders/TextureFlipFrag.frag:FRAGMENT"
        },
        "TextureFlipShader"
    );
    mResources_.loadResource<clay::ShaderProgram>(
        {
            "Shaders/Solid.vert:VERTEX",
            "Shaders/Solid.frag:FRAGMENT"
        },
        "SolidShader"
    );
    // Meshes
    mResources_.loadResource<clay::Mesh>({"Models/Plane.obj"},"Plane");
    mResources_.loadResource<clay::Mesh>({"Models/GloveRight.obj"},"GloveRight");
    mResources_.loadResource<clay::Mesh>({"Models/GloveLeft.obj"},"GloveLeft");
    mResources_.loadResource<clay::Mesh>({"Models/Cube.obj"},"Cube");
    mResources_.loadResource<clay::Mesh>({"Models/Sphere.obj"},"Sphere");
    // audio
    mResources_.loadResource<clay::Audio>({"Audio/beep_deep_1.wav"}, "DeepBeep");
    // fonts
    mResources_.loadResource<clay::Font>({"Fonts/Consolas.ttf"}, "Consolas");
    mResources_.loadResource<clay::Font>({"Fonts/runescape_uf.ttf"}, "Runescape");
    // Texture
    mResources_.loadResource<clay::Texture>({"Textures/V.png"}, "VTexture");
    mResources_.loadResource<clay::Texture>({"Textures/earthmap1k.jpg"}, "Earth1k");
    mResources_.loadResource<clay::Texture>({"Textures/2k_moon.jpg"}, "Moon2k");
    mResources_.loadResource<clay::Texture>({"Textures/sunmap.jpg"}, "Sun");
    mResources_.loadResource<clay::Texture>({"Textures/8k_stars_milky_way.jpg"}, "Galaxy");
    mResources_.loadResource<clay::Texture>({"Textures/CloudSky.jpg"}, "CloudSky");
    mResources_.loadResource<clay::Texture>({"Textures/FarmPreview.jpg"}, "FarmPreview");
    mResources_.loadResource<clay::Texture>({"Textures/SandboxPreview.jpg"}, "SandboxPreview");
    mResources_.loadResource<clay::Texture>({"Textures/SpacePreview.jpg"}, "SpacePreview");

    // Manually build a mesh
    mResources_.addResource(
        std::move(std::make_unique<clay::Mesh>(
            *mGraphicsAPI_,
            std::vector<clay::Mesh::Vertex>{
                // Position, normal, UV, tangent, bitangent
                {{-.5f, -.5f, 0.0f}, {0.0f,  0.0f, -1.0f},  {0.0f,  0.0f},{0,0,0},{0,0,0}},
                {{.5f, -.5f, 0.0f}, {0.0f,  0.0f, -1.0f},  {1.0f,  0.0f},{0,0,0},{0,0,0}},
                {{.5f,  .5f, 0.0f}, {0.0f,  0.0f, -1.0f},  {1.0f,  1.0f},{0,0,0},{0,0,0}},
                {{.5f,  .5f, 0.0f}, {0.0f,  0.0f, -1.0f},  {1.0f,  1.0f},{0,0,0},{0,0,0}},
                {{-.5f,  .5f, 0.0f}, {0.0f,  0.0f, -1.0f},  {0.0f,  1.0f},{0,0,0},{0,0,0}},
                {{-.5f, -.5f, 0.0f}, {0.0f,  0.0f, -1.0f},  {0.0f,  0.0f},{0,0,0},{0,0,0}},
            },
            std::vector<unsigned int>{
                0,1,2,
                3,4,5,
            }
        )),
        "RectPlane"
    );
}

void DemoAppXR::initialize() {
    AppXR::initialize();
    // build scenes details
    mSceneDetails_ = {
        {
            "Sandbox Scene",
            "Scene for experimenting features.",
            mResources_.getResource<clay::Texture>("SandboxPreview")->getId(),
            [this]{
                mScenes_.front()->setRemove(true);
                setScene(new SandboxScene(*this));
            }
        },
        {
            "Space Scene",
            "Space simulation with an orbiting solar system.",
            mResources_.getResource<clay::Texture>("SpacePreview")->getId(),
            [this]{
                mScenes_.front()->setRemove(true);
                setScene(new SpaceScene(*this));
            }
        },
        {
            "Farm Scene",
            "Simple Nature Scene with future plans to include farming.",
            mResources_.getResource<clay::Texture>("FarmPreview")->getId(),
            [this]{
                mScenes_.front()->setRemove(true);
                setScene(new FarmScene(*this));
            }
        }
    };
}

std::vector<SceneDetail> &DemoAppXR::getSceneDetails() {
    return mSceneDetails_;
}

