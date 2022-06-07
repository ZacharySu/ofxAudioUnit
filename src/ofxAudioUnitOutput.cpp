#include "ofxAudioUnitOutput.h"
#include "ofxAudioUnitUtils.h"
#include "ofxAudioUnitHardwareUtils.h"
#include <CoreFoundation/CoreFoundation.h>
#define kOutputBus 0
#define kInputBus 1
using namespace std;

static OSStatus RenderAndCopy(void * inRefCon,
                              AudioUnitRenderActionFlags * ioActionFlags,
                              const AudioTimeStamp * inTimeStamp,
                              UInt32 inBusNumber,
                              UInt32 inNumberFrames,
                              AudioBufferList * ioData);

AudioComponentDescription outputDesc = {
	kAudioUnitType_Output,
    kAudioUnitSubType_VoiceProcessingIO,
	kAudioUnitManufacturer_Apple
};

typedef std::shared_ptr<AudioBufferList> AudioBufferListRef;
struct OutputContext
{
    AudioBufferListRef bufferList;
};

// ----------------------------------------------------------
ofxAudioUnitOutput::ofxAudioUnitOutput()
: ctx(new OutputContext)
// ----------------------------------------------------------
{
	_desc = outputDesc;
    outputIsRunning = false;
	initUnit();
}

// ----------------------------------------------------------
bool ofxAudioUnitOutput::start()
// ----------------------------------------------------------
{
//    AURenderCallbackStruct callback = {RenderAndCopy, this};
//    setRenderCallback(callback, 0);
    
	OFXAU_RET_BOOL(AudioOutputUnitStart(*_unit), "starting output unit");
    
    //-----------------------------------
//    AudioStreamBasicDescription outputASDB;
//        UInt32  outputASDBSize = sizeof(outputASDB);
//    OFXAU_RET_FALSE(AudioUnitGetProperty(*_unit, kAudioUnitProperty_StreamFormat, kAudioUnitScope_Output, 0, &outputASDB,&outputASDBSize),"get property fail");
//
//    outputIsRunning = true;
//    ((OutputContext*)ctx)->bufferList = AudioBufferListRef(AudioBufferListAlloc(outputASDB.mChannelsPerFrame, 1024), AudioBufferListRelease);
//    if(pthread_create(&renderTid, NULL, RenderingThread, (void *)this) != 0){
//        assert(!"pthread library error");
//    }

    return true;
}

// ----------------------------------------------------------
bool ofxAudioUnitOutput::stop()
// ----------------------------------------------------------
{
	OFXAU_RET_BOOL(AudioOutputUnitStop(*_unit), "stopping output unit");
//    if(outputIsRunning){
//        outputIsRunning = false;
//        int ret = pthread_join(renderTid, NULL);
//        if (ret != 0) {
//            cout << "pthread_join error ret" << ret << endl;
//            assert(!"pthread_join error");
//        }
//    }
    return true;
}

void* ofxAudioUnitOutput::RenderingThread(void *arg){
    ofxAudioUnitOutput *output = (ofxAudioUnitOutput *)arg;
    OutputContext *ctx = (OutputContext *)output->ctx;
    AudioUnitRenderActionFlags ioActionFlags = 0;
    AudioTimeStamp inTimeStamp;
    memset(&inTimeStamp, 0, sizeof(inTimeStamp));
    inTimeStamp.mFlags = kAudioTimeStampSampleTimeValid;
    inTimeStamp.mSampleTime = 0;
    
    AudioStreamBasicDescription outputASDB;
        UInt32  outputASDBSize = sizeof(outputASDB);
    OFXAU_PRINT(AudioUnitGetProperty(output->getUnit(), kAudioUnitProperty_StreamFormat, kAudioUnitScope_Output, 0, &outputASDB,&outputASDBSize),"get property fail");
    AudioUnitRenderActionFlags flags = kAudioUnitRenderAction_OutputIsSilence;
    UInt32 busNumber = 0;
    UInt32 numberFrames = 512;
    int channelCount = 1;
    
    while (output->outputIsRunning) {
        OFXAU_PRINT(AudioUnitRender(output->getUnit(),
                                     &outputASDB.mFormatFlags,
                                     &inTimeStamp,
                                     0,
                                     512,
                                     ctx->bufferList.get()), "audio unit output redner error");
    }
    return nullptr;
}

//OSStatus RenderAndCopy(void * inRefCon,
//                       AudioUnitRenderActionFlags * ioActionFlags,
//                       const AudioTimeStamp * inTimeStamp,
//                       UInt32 inBusNumber,
//                       UInt32 inNumberFrames,
//                       AudioBufferList * ioData)
//{
//    ofxAudioUnitOutput * render = static_cast<ofxAudioUnitOutput *>(inRefCon);
//    OSStatus status = noErr;
//    std::cout << "render" << std::endl;
//    return status;
//}
