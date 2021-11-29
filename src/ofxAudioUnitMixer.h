#pragma once

#include "ofxAudioUnitBase.h"

// ofxAudioUnitMixer wraps the AUMultiChannelMixer
// This is a multiple-input, single-output mixer.
// Call setInputBusCount() to change the number
// of inputs on the mixer.

// You can use this unit to get access to the level
// of the audio going through it (in decibels).
// First call enableInputMetering() or
// enableOutputMetering() (depending on which one
// you're interested in). After this, getInputLevel()
// or getOutputLevel() will return a float describing
// the current level in decibles (most likely in the
// range -120 - 0)

class ofxAudioUnitMixer : public ofxAudioUnit
{
public:
	ofxAudioUnitMixer();
    ofxAudioUnitMixer(int sampleRate, int channels, int bitDepth);
	void setInputVolume (float volume, int bus = 0);
	void setOutputVolume(float volume);
	void setPan(float pan, int bus = 0);
    void setOutputASBD(int sampleRate, int channels, int bitDepth);
    void setOutputCallback(void *impl, void (callback)(void* impl, uint8_t *data, int sampleRate, int channels, int depth, int length));
    
	float getInputLevel(int bus = 0);
	float getOutputLevel() const;
    void* getOutputImpl();
	bool setInputBusCount(unsigned int numberOfInputBusses);
	unsigned int getInputBusCount() const;
	
	void  enableInputMetering(int bus = 0);
	void  enableOutputMetering();
	void  disableInputMetering(int bus = 0);
	void  disableOutputMetering();
    
    //private
    int outputSampleRate;
    int outputChannels;
    int outputBitDepth;
    void (*outputCallback)(void* impl, uint8_t *data, int sampleRate, int channels, int depth, int length);
private:
    AudioStreamBasicDescription outputASBD;
    void* outputImpl;
};
