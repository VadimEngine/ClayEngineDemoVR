#include "AudioManager.h"

AudioManager::AudioManager() {
    mSoundDevice_ = new SoundDevice();
    mSoundSource_ = new SoundSource();
}

AudioManager::~AudioManager() {
    delete mSoundSource_;
    delete mSoundDevice_;
}

void AudioManager::playSound(ALuint audioId) {
    mSoundSource_->play(audioId);
}

void AudioManager::setGain(float newGain) {
    mSoundSource_->setGain(newGain);
}

float AudioManager::getGain() const {
    return mSoundSource_->getGain();
}
