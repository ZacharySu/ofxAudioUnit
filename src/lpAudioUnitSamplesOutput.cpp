//
//  lpAudioUnitSamplesOutput.cpp
//  LQAudioUnit
//
//  Created by suzhou on 2021/11/29.
//

#include "lpAudioUnitSamplesOutput.hpp"
#include "ofxAudioUnitUtils.h"

// a passthru render callback which copies the rendered samples in the process
static OSStatus RenderAndCopy(void * inRefCon,
                              AudioUnitRenderActionFlags *    ioActionFlags,
                              const AudioTimeStamp *    inTimeStamp,
                              UInt32 inBusNumber,
                              UInt32    inNumberFrames,
                              AudioBufferList * ioData);

lpAudioUnitSamplesOutput::lpAudioUnitSamplesOutput(){
    setup(kAudioUnitType_FormatConverter, kAudioUnitSubType_AUConverter);
}
lpAudioUnitSamplesOutput::~lpAudioUnitSamplesOutput(){
    
}
    
// ----------------------------------------------------------
void lpAudioUnitSamplesOutput::setOutputASBD(int sampleRate, int channels, int bitDepth)
// ----------------------------------------------------------
{
    outputSampleRate = sampleRate;
    outputChannels = channels;
    outputBitDepth = bitDepth;
    
    UInt32 size = sizeof(outputASBD);
    // set output stream format and callback for mixer
    AudioUnitGetProperty(*_unit, kAudioUnitProperty_StreamFormat, kAudioUnitScope_Output, 0, &outputASBD, &size);
    
    outputASBD.mSampleRate = sampleRate;
    outputASBD.mFormatID = kAudioFormatLinearPCM;
    outputASBD.mFormatFlags = kAudioFormatFlagIsSignedInteger | kAudioFormatFlagsNativeEndian | kAudioFormatFlagIsPacked;
    outputASBD.mFramesPerPacket = 1;
    outputASBD.mChannelsPerFrame = channels;
    outputASBD.mBitsPerChannel = bitDepth;
    outputASBD.mBytesPerPacket = bitDepth/8 * channels;
    outputASBD.mBytesPerFrame = bitDepth/8 * channels;
    
    OFXAU_PRINT(AudioUnitSetProperty(*_unit,
                                      kAudioUnitProperty_StreamFormat,
                                      kAudioUnitScope_Output,
                                      0,
                                      &outputASBD,
                                      size),
                "setting mixer output gain");
}
// ----------------------------------------------------------
void lpAudioUnitSamplesOutput::setOutputCallback(void *impl, void (callback)(void *impl, uint8_t *data, int sampleRate, int channels, int depth, int length))
// ----------------------------------------------------------
{
    outputCallback = callback;
    outputImpl = impl;
    OFXAU_PRINT(AudioUnitAddRenderNotify(*_unit,
                                         &RenderAndCopy, this),
                "Mixer setOutputCallback");
}


void *lpAudioUnitSamplesOutput::getOutputImpl(){
    return outputImpl;
}

void lpAudioUnitSamplesOutput:: startProcess(){
    
}
void lpAudioUnitSamplesOutput:: stopProcess(){
    
}
// ----------------------------------------------------------
OSStatus RenderAndCopy(void * inRefCon,
                       AudioUnitRenderActionFlags * ioActionFlags,
                       const AudioTimeStamp * inTimeStamp,
                       UInt32 inBusNumber,
                       UInt32 inNumberFrames,
                       AudioBufferList * ioData)
{
    OSStatus status;
    
    if ((*ioActionFlags & kAudioUnitRenderAction_PostRender) && inBusNumber == 0)
    {
        lpAudioUnitSamplesOutput* impl = (lpAudioUnitSamplesOutput*)inRefCon;
//        impl->printAudioUnitASBD(impl->getUnit());
          
        for (int bufferIndex = 0; bufferIndex < ioData->mNumberBuffers; bufferIndex++) {
            unsigned char *audioBufferPtr = (unsigned char *)ioData->mBuffers[bufferIndex].mData;
            int size = (int)ioData->mBuffers[bufferIndex].mDataByteSize;
//            std::cout << bufferIndex << ":" << size << std::endl;
            if(impl->outputCallback){
                impl->outputCallback(impl->getOutputImpl(), audioBufferPtr, impl->outputSampleRate, impl->outputChannels, impl->outputBitDepth, size);
            }
        }
    }
    
    return status;
}
