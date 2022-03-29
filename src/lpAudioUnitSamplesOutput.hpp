//
//  lpAudioUnitSamplesOutput.hpp
//  LPAudioUnit
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
    void setOutputSize(unsigned int size);
    bool startProcess();
    void stopProcess();
    
private:
    struct PCMOutputImpl;
    std::shared_ptr<PCMOutputImpl> _impl;
    AUGraph         auGraph;
    AudioStreamBasicDescription outputASBD;
    bool outputIsRunning;
};
#endif /* lpAudioUnitSamplesOutput_hpp */
