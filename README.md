This a template to build android APK using Gradle through CLI. This project can be copied and edited to fit a new project or the following steps can be run through to build a new project from scratch

## Build from Scratch with Gradle
- cd to project 
- gradle init
- create ./app/src/main/AndroidManifest.xml
    - Copy from template to fill in details
- mkdir -p app/src/main/java/com/example/{APPNAME}
- mkdir -p app/src/main/res/layout
- mkdir -p app/src/main/res/values
- mkdir -p app/src/main/assets
- mkdir -p app/src/main/jniLibs
- create ./build.gradle
    - Copy from template to fill in details
- Update ./app/build.gradle to be for android
- Create MainActivity.java
    - touch ./app/src/main/java/com/example/{APPName}/MainActivity.java
    - copy from template
- Update ./gradle.properties to match template
- create Create app/src/main/res/layout/activity_main.xml:
    - touch app/src/main/res/layout/activity_main.xml
    - Copy from template


## Copy from Template
- copy all but build
- rename following to app name
    - ./settings.gradle
        - rootProject.name = '<APPNAME>'
   - ./app/src/main/AndroidManifest.xml
        -package="com.example.<APPNAME>>">
    - ./app/src/main/cpp/app.cpp
        - rename all instances of "Template" to <APPNAME>
    - ./app/src/main/headers\app.h
        - rename all instances of "Template" to <APPNAME>
    - ./app/src/main/java/com/example/Template/MainActivity.java
        - package com.example.<APPNAME>>;
    - ./app/src/test/cpp/app_test.cpp
        - rename all instances of "Template" to <APPNAME>
- run build


### Build:
- ./gradlew assembleDebug
- ./gradlew build
- output is in ./app/build/outputs/apk/debug/app-debug.apk

Upload to phone
- adb install path/to/your_app.apk