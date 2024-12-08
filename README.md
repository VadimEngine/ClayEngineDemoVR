# ClayEngine Demo VR

Work in progress Oculus VR application. Make using OpenXR with Android and OpenGL-ES and parts of ClayEngine (https://github.com/VadimEngine/ClayEngine). This application allows loading and using resources to build scenes that the use can navigate and interact with.

### Third Party Libraries:
- assimp
    - Used to load 3d models to render in the scene. This is combined with android asset manager to load the Model resources which are passed to assimp api to build a Model that can be rendered.
- freetype
    - Used to load fonts. This is combined with android asset manager to load the font resources which are passed to freeType api to build a Font that can be used to render text
- imgui
    - Used to build and render GUIs in the scene. ClayEngine Demo VR allows rendering imgui onto a plane and map the controller's aim input to interact with the gui
- libsndfile
    - Used to Convert .wav file data that is loaded using android asset manager into audio data that can be used with OpenAL to play audio
- openal-soft
    - Used to play audio in the application
- OpenXR-SDK-Source
    - Used to interact with the VR device (in this case, Oculus) to render content and get device input/output with the headset and controllers.



### Sandbox Scene
![alt text](./Screenshots/Scene1.png)

Sandbox scene renders rods at the user's controller's aim direction. A loaded 3d model with is rendered with a loaded texture. A sample string is rendered with a loaded font. And an Imgui window is rendered unto a plane and can be interacted with controller. The gui displays the user's controller and headset state. A sample audio can be played with a button and the scene can be swapped

### Space Scene
![alt text](./Screenshots/Scene2.png)

Similar to Sandbox scene but instead, a sun with an orbiting planet is displayed

### Build

build CLI:
- `./gradlew clean assembleDebug`
- `./gradlew clean build`

The build `app-debug.apk` will be `ClayEngineDemoVR\app\build\intermediates\apk\debug\app-debug.apk` can be deployed to an Oculus device

Alternatively, this can be build with Android studio.

### Upcoming Plans
- More scenes are planned to be added and ClayEngine is planned to fully be included after it is updated to support developing with OpenXR.
