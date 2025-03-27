# ClayEngine Demo VR

Demo application of using the ClayEngine static library (https://github.com/VadimEngine/ClayEngine) to build a VR application with Android and OpenGLES. This application allows loading and using resources to build scenes that the user can navigate and interact with. This application primarily uses OpenXR from ClayEngine for the VR functionality

![alt text](./Screenshots/VRDemoGif.gif)

## Scenes

### Sandbox Scene

Sandbox scene renders hands at the user's controller's. A loaded 3d model with a loaded texture and a sample string is rendered with a loaded font. And an Imgui window is rendered onto a plane and can be interacted with controller. The gui displays the user's controller and headset state. A sample audio can be played with a button and the other scenes can be previewed and switched to.

### Space Scene

Similar to Sandbox scene but instead, a sun with an orbiting planet and moon is rendered

### Farm Scene

Similar to Sandbox scene but instead, a nature scene is rendered. 

## Build

build CLI:
- `./gradlew clean assembleDebug`
- `./gradlew clean build`

The build `app-debug.apk` will be `ClayEngineDemoVR\app\build\intermediates\apk\debug\app-debug.apk` can be deployed to an Oculus device

Alternatively, this can be built and deployed with Android studio.
