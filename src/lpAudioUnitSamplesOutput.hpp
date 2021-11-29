//
//  lpAudioUnitSamplesOutput.hpp
//  LQAudioUnit
//
//  Created by suzhou on 2021/11/29.
//

#ifndef lpAudioUnitSamplesOutput_hpp
#define lpAudioUnitSamplesOutput_hpp

#include <stdio.h>

#include "ofxAudioUnitBase.h"

class lpAudioUnitSamplesOutput: public ofxAudioUnit {
public:
    lpAudioUnitSamplesOutput();
    ~lpAudioUnitSamplesOutput();
    
    void setOutputASBD(int sampleRate, int channels, int bitDepth);
    void setOutputCallback(void *impl, void (callback)(void* impl, uint8_t *data, int sampleRate, int channels, int depth, int length));
    
    void* getOutputImpl();
    
    void startProcess();
    void stopProcess();
    
    //private
    int outputSampleRate;
    int outputChannels;
    int outputBitDepth;
    void (*outputCallback)(void* impl, uint8_t *data, int sampleRate, int channels, int depth, int length);
    
private:
    
    AudioStreamBasicDescription outputASBD;
    void* outputImpl;
};
#endif /* lpAudioUnitSamplesOutput_hpp */
