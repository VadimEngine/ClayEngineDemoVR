#include "Audio.h"

sf_count_t vio_get_filelen(void* user_data) {
    VirtualFile* vf = static_cast<VirtualFile*>(user_data);
    return vf->size;
}

// Read function
sf_count_t vio_read(void* ptr, sf_count_t count, void* user_data) {
    VirtualFile* vf = static_cast<VirtualFile*>(user_data);
    sf_count_t bytesToRead = std::min(count, vf->size - vf->pos);
    std::memcpy(ptr, vf->data + vf->pos, bytesToRead);
    vf->pos += bytesToRead;
    return bytesToRead;
}

// Seek function
sf_count_t vio_seek(sf_count_t offset, int whence, void* user_data) {
    VirtualFile* vf = static_cast<VirtualFile*>(user_data);
    sf_count_t newPos = vf->pos;

    if (whence == SEEK_SET) newPos = offset;
    else if (whence == SEEK_CUR) newPos += offset;
    else if (whence == SEEK_END) newPos = vf->size + offset;

    if (newPos < 0 || newPos > vf->size) return -1; // Out of bounds
    vf->pos = newPos;
    return vf->pos;
}

// Tell function
sf_count_t vio_tell(void* user_data) {
    VirtualFile* vf = static_cast<VirtualFile*>(user_data);
    return vf->pos;
}

// No-op write function (reading only)
sf_count_t vio_write(const void* ptr, sf_count_t count, void* user_data) {
    return 0;
}

int Audio::loadAudio(AAssetManager* assetManager, const std::string& filePath) {
    ALenum err, format;
    ALuint buffer;
    SNDFILE* sndfile;
    SF_INFO sfinfo;
    short* membuf;
    sf_count_t num_frames;
    ALsizei num_bytes;

    auto fileData = xr::utils::loadAsset(assetManager, filePath);

    // Open the audio file and check that it's usable.
    SF_VIRTUAL_IO vio = {
            vio_get_filelen, // Function to get the length of the file
            vio_seek,        // Function to seek within the file
            vio_read,        // Function to read from the file
            vio_write,       // Function to write to the file (unused in this case)
            vio_tell         // Function to get the current position in the file
    };
    VirtualFile vf = { fileData.data(), static_cast<sf_count_t>(fileData.size()), 0 };

    sndfile = sf_open_virtual(&vio, SFM_READ, &sfinfo, &vf);
    if (!sndfile) {
        fprintf(stderr, "Error: %s\n", sf_strerror(NULL));
        return 0;
    }

    if (sfinfo.frames < 1 || sfinfo.frames >(sf_count_t)(INT_MAX / sizeof(short)) / sfinfo.channels) {
        fprintf(stderr, "Bad sample count in %s (%" PRId64 ")\n", filePath.c_str(), sfinfo.frames);
        sf_close(sndfile);
        return 0;
    }

    // Get the sound format, and figure out the OpenAL format
    format = AL_NONE;
    if (sfinfo.channels == 1)
        format = AL_FORMAT_MONO16;
    else if (sfinfo.channels == 2)
        format = AL_FORMAT_STEREO16;
    else if (sfinfo.channels == 3) {
        if (sf_command(sndfile, SFC_WAVEX_GET_AMBISONIC, NULL, 0) == SF_AMBISONIC_B_FORMAT)
            format = AL_FORMAT_BFORMAT2D_16;
    }
    else if (sfinfo.channels == 4) {
        if (sf_command(sndfile, SFC_WAVEX_GET_AMBISONIC, NULL, 0) == SF_AMBISONIC_B_FORMAT)
            format = AL_FORMAT_BFORMAT3D_16;
    }
    if (!format) {
        fprintf(stderr, "Unsupported channel count: %d\n", sfinfo.channels);
        sf_close(sndfile);
        return 0;
    }

    // Decode the whole audio file to a buffer.
    membuf = static_cast<short*>(malloc((size_t)(sfinfo.frames * sfinfo.channels) * sizeof(short)));

    num_frames = sf_readf_short(sndfile, membuf, sfinfo.frames);
    if (num_frames < 1) {
        free(membuf);
        sf_close(sndfile);
        fprintf(stderr, "Failed to read samples in %s (%" PRId64 ")\n", filePath.c_str(), num_frames);
        return 0;
    }
    num_bytes = (ALsizei)(num_frames * sfinfo.channels) * (ALsizei)sizeof(short);

    // Buffer the audio data into a new buffer object, then free the data and close the file.
    buffer = 0;
    alGenBuffers(1, &buffer);
    alBufferData(buffer, format, membuf, num_bytes, sfinfo.samplerate);

    free(membuf);
    sf_close(sndfile);

    // Check if an error occurred, and clean up if so.
    err = alGetError();
    if (err != AL_NO_ERROR) {
        fprintf(stderr, "OpenAL Error: %s\n", alGetString(err));
        if (buffer && alIsBuffer(buffer))
            alDeleteBuffers(1, &buffer);
        return 0;
    }

    return buffer;
}

Audio::Audio(ALuint audioId)
    : mId_(audioId) {}

Audio::~Audio() {
    alDeleteBuffers(1, &mId_);
}

int Audio::getId() const {
    return mId_;
}
