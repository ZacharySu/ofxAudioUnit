//
//  ofxAudioUnitRawInput.hpp
//  LPAudioUnit
//
//  Created by suzhou on 2022/5/31.
//

#ifndef ofxAudioUnitRawInput_hpp
#define ofxAudioUnitRawInput_hpp

#include <stdio.h>
#include "ofxAudioUnitBase.h"
class ofxAudioUnitRawInput: public ofxAudioUnit{
public:
    ofxAudioUnitRawInput();
    ~ofxAudioUnitRawInput();
    void updateAudioPcmBuffer(AudioStreamBasicDescription ASBD, void *data, int byteSize);
private:
    void setInputStreamFormat(AudioStreamBasicDescription ASBD);
    void updateAudioPcmBuffer(void *data, int byteSize);
    struct RawInputImpl;
    std::shared_ptr<RawInputImpl> _impl;
};
#endif /* ofxAudioUnitRawInput_hpp */
