#pragma once

#include "ofxAudioUnitMixer.h"

// ofxAudioUnitRawMixer wraps the ofxAudioUnitMixer
// This is a raw pcm mix

class ofxAudioUnitRawMixer : public ofxAudioUnitMixer
{
public:
    ofxAudioUnitRawMixer();
    ~ofxAudioUnitRawMixer();
    bool setInputBusCount(unsigned int numberOfInputBusses, unsigned int samplesToBuffer = 2048);
    void setInputStreamFormat(int sampleRate, int channels, int bitDepth, uint32_t destinationBus = 0);
    void setInputStreamFormat(AudioStreamBasicDescription ASBD, uint32_t destinationBus);
    void updateAudioPcmBuffer(void *data, int byteSize, uint32_t destinationBus);
    void updateAudioPcmBuffer(AudioStreamBasicDescription ASBD, void *data, int byteSize, uint32_t destinationBus);
private:
    struct RawMixerImpl;
    std::shared_ptr<RawMixerImpl> _impl;
};
