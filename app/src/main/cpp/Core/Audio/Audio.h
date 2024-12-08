#pragma once
// standard lib
#include <inttypes.h>
#include <string>
#include <vector>
// third party
#include <AL/alext.h>
#include <sndfile.h>
// project
#include "XR/XRUtils.h"

// State for virtual I/O
struct VirtualFile {
    const char* data;
    sf_count_t size;
    sf_count_t pos;
};

class Audio {
public:
    static int loadAudio(AAssetManager* assetManager, const std::string& filePath);

    Audio(ALuint audioId);

    ~Audio();

    /** Get the AL buffer id */
    int getId() const;

private:
    /** AL audio id */
    ALuint mId_;
};