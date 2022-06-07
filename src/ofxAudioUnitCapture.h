#pragma once

#include "ofxAudioUnitBase.h"

class ofxAudioUnitCapture : public ofxAudioUnit
{
public:
	ofxAudioUnitCapture(unsigned int samplesToBuffer = 2048);
	~ofxAudioUnitCapture();
	
	ofxAudioUnit& connectTo(ofxAudioUnit &otherUnit, int destinationBus = 0, int sourceBus = 0);
	using ofxAudioUnit::connectTo; // for connectTo(ofxAudioUnitTap&)
	
	OSStatus render(AudioUnitRenderActionFlags *ioActionFlags,
					const AudioTimeStamp *inTimeStamp,
					UInt32 inOutputBusNumber,
					UInt32 inNumberFrames,
					AudioBufferList *ioData);
	
	UInt32 getNumOutputChannels() const;
	
	bool start();
	bool stop();
	
#if !TARGET_OS_IPHONE
	bool setDevice(AudioDeviceID deviceID);
	bool setDevice(const std::string &deviceName);
	
	static void listInputDevices();
#endif
	
private:
	struct InputImpl;
	std::shared_ptr<InputImpl> _impl;
	bool configureInputDevice();
};
