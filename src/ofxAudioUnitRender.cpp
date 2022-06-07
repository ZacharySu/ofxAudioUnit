#include "ofxAudioUnitRender.h"
#include "ofxAudioUnitUtils.h"
#include "ofxAudioUnitHardwareUtils.h"
#include <CoreFoundation/CoreFoundation.h>

AudioComponentDescription renderDesc = {
	kAudioUnitType_Output,
#if TARGET_OS_IPHONE
	kAudioUnitSubType_RemoteIO,
#else
	kAudioUnitSubType_HALOutput,
#endif
	kAudioUnitManufacturer_Apple
};


// ----------------------------------------------------------
ofxAudioUnitRender::ofxAudioUnitRender()
// ----------------------------------------------------------
{
	_desc = renderDesc;
	initUnit();
    AudioStreamBasicDescription outputASBD;
    UInt32 size = sizeof(outputASBD);
    // set output stream format and callback for mixer
    AudioUnitGetProperty(*_unit, kAudioUnitProperty_StreamFormat, kAudioUnitScope_Output, 0, &outputASBD, &size);
//    printASBD(outputASBD);
}

// ----------------------------------------------------------
bool ofxAudioUnitRender::start()
// ----------------------------------------------------------
{
    printAudioUnitASBD(*_unit, false);
	OFXAU_RET_BOOL(AudioOutputUnitStart(*_unit), "starting output unit");
}

// ----------------------------------------------------------
bool ofxAudioUnitRender::stop()
// ----------------------------------------------------------
{
	OFXAU_RET_BOOL(AudioOutputUnitStop(*_unit), "stopping output unit");
}

#pragma mark - Hardware

#if !TARGET_OS_IPHONE

// ----------------------------------------------------------
bool ofxAudioUnitRender::setDevice(AudioDeviceID deviceID)
// ----------------------------------------------------------
{
	UInt32 deviceIDSize = sizeof(deviceID);
	OFXAU_RET_BOOL(AudioUnitSetProperty(*_unit,
										kAudioOutputUnitProperty_CurrentDevice,
										kAudioUnitScope_Global,
										0,
										&deviceID,
										deviceIDSize),
				   "setting output unit's device ID");
}

// ----------------------------------------------------------
bool ofxAudioUnitRender::setDevice(const std::string &deviceName)
// ----------------------------------------------------------
{
	std::vector<AudioDeviceID> outputDevices = AudioOutputDeviceList();
    AudioDeviceID deviceID{0};
	bool found = false;
	for(int i = 0; i < outputDevices.size(); i++) {
		int diff = AudioDeviceName(outputDevices[i]).compare(deviceName);
		if(!diff) {
			deviceID = outputDevices[i];
			found = true;
			break;
		}
	}
	
	if(found) {
		return setDevice(deviceID);
	} else {
		return false;
	}
}

// ----------------------------------------------------------
void ofxAudioUnitRender::listOutputDevices()
// ----------------------------------------------------------
{
	std::vector<AudioDeviceID> deviceList = AudioOutputDeviceList();
	
	for(int i = 0; i < deviceList.size(); i++) {
		FLog("ID[%d]  \tName[%s]", deviceList[i], AudioDeviceName(deviceList[i]).c_str());
	}
}

#endif // !TARGET_OS_IPHONE
