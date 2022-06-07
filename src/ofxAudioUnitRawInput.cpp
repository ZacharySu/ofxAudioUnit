//
//  ofxAudioUnitRawInput.cpp
//  LPAudioUnit
//
//  Created by suzhou on 2022/5/31.
//

#include "ofxAudioUnitRawInput.h"
#include "ofxAudioUnitUtils.h"

#include "TPCircularBuffer.h"

using namespace std;
static OSStatus RenderAndCopy(void * inRefCon,
                              AudioUnitRenderActionFlags * ioActionFlags,
                              const AudioTimeStamp * inTimeStamp,
                              UInt32 inBusNumber,
                              UInt32 inNumberFrames,
                              AudioBufferList * ioData);
struct RawInputContex{
    AudioStreamBasicDescription asbd;
    TPCircularBuffer circularBuffer;
    std::mutex bufferMutex;
    uint64_t inputStatisticCount = 200;
    uint64_t outputStatisticCount = 1000;
    RawInputContex():
    asbd({0}),
    _bufferSize(0){
        
    }
    UInt32 getRawInputSize(){
        return _bufferSize;
    }
    void setRawInputSize(UInt32 samplesToBuffer){
        if(samplesToBuffer != _bufferSize) {
            bufferMutex.lock();
            {
                TPCircularBufferCleanup(&circularBuffer);
                TPCircularBufferInit(&circularBuffer, samplesToBuffer * sizeof(Float32));
                _bufferSize = samplesToBuffer;
            }
            bufferMutex.unlock();
        }
    }
    bool asbdIsChange(AudioStreamBasicDescription _targetASBD){
        bufferMutex.lock();
        if(memcmp(&_targetASBD, &asbd, sizeof(AudioStreamBasicDescription))){
            memcpy(&asbd, &_targetASBD, sizeof(AudioStreamBasicDescription));
            TPCircularBufferClear(&circularBuffer);
            FLog("ofxAudioUnitRawInput changed to");
            bufferMutex.unlock();
            return true;
        }
        bufferMutex.unlock();
        return false;
    }
private:
    UInt32 _bufferSize;
};
struct ofxAudioUnitRawInput::RawInputImpl{
    RawInputContex ctx;
};

ofxAudioUnitRawInput:: ofxAudioUnitRawInput(): _impl(std::make_shared<RawInputImpl>()){
    setup(kAudioUnitType_FormatConverter, kAudioUnitSubType_AUConverter);
}
ofxAudioUnitRawInput::~ofxAudioUnitRawInput(){
    TPCircularBufferCleanup(&_impl->ctx.circularBuffer);
}
void ofxAudioUnitRawInput::setInputStreamFormat(AudioStreamBasicDescription ASBD){
    printAudioUnitASBD(*_unit, true);
    FLog("To:\n");
    printASBD(ASBD);
    std::string errorMsg = "setting Raw input ASBD destination format ";
    OFXAU_RETURN(AudioUnitSetProperty(*_unit,
                                      kAudioUnitProperty_StreamFormat,
                                      kAudioUnitScope_Input,
                                      0,
                                      &ASBD,
                                      sizeof(ASBD)),
                 errorMsg.c_str());
    AURenderCallbackStruct callback = {RenderAndCopy, &_impl->ctx};
    setRenderCallback(callback);
}
void ofxAudioUnitRawInput::updateAudioPcmBuffer(void *data, int byteSize){
    if(_impl->ctx.getRawInputSize() < byteSize){
        _impl->ctx.setRawInputSize(byteSize);
    }
    if(_impl->ctx.bufferMutex.try_lock()) {
        int32_t availableBytesInCircBuffer;
        TPCircularBufferHead(&_impl->ctx.circularBuffer, &availableBytesInCircBuffer);
        
        if(availableBytesInCircBuffer < byteSize) {
            TPCircularBufferConsume(&_impl->ctx.circularBuffer, byteSize - availableBytesInCircBuffer);
            if(0 == _impl->ctx.inputStatisticCount++%200){
                FLog("音频数据超出，将删除：%d", (byteSize - availableBytesInCircBuffer));
            }
        }
        
        TPCircularBufferProduceBytes(&_impl->ctx.circularBuffer, data, byteSize);
        _impl->ctx.bufferMutex.unlock();
    }
}
void ofxAudioUnitRawInput::updateAudioPcmBuffer(AudioStreamBasicDescription ASBD, void *data, int byteSize){
    if(_impl->ctx.asbdIsChange(ASBD)){
        setInputStreamFormat(ASBD);
    }
    updateAudioPcmBuffer(data, byteSize);
}
OSStatus RenderAndCopy(void * inRefCon,
                       AudioUnitRenderActionFlags * ioActionFlags,
                       const AudioTimeStamp * inTimeStamp,
                       UInt32 inBusNumber,
                       UInt32 inNumberFrames,
                       AudioBufferList * ioData)
{
    OSStatus status = noErr;
    RawInputContex * ctx = static_cast<RawInputContex *>(inRefCon);
    assert(inBusNumber == 0);
    if(ctx->bufferMutex.try_lock()) {
        int32_t availableBytes = 0;
        UInt32 sizeInNeed = 0;
        TPCircularBufferTail(&ctx->circularBuffer, &availableBytes);
        
        for(int i = 0; i < ioData->mNumberBuffers; i++){
            sizeInNeed += ioData->mBuffers[i].mDataByteSize;
        }
        if(sizeInNeed > 0 && availableBytes >= sizeInNeed){
            for(int i = 0; i < ioData->mNumberBuffers; i++){
                int bufferToConsume = TPCircularBufferGetData(&ctx->circularBuffer, ioData->mBuffers[i].mData, ioData->mBuffers[i].mDataByteSize);
//                ctx->outputStatisticCount++;
//                if(0 == ctx->outputStatisticCount % 1000){
//                    FLog("%d消费了音频数据：%d",inBusNumber, bufferToConsume);
//                }
            }
        } else {
            for(int i = 0; i < ioData->mNumberBuffers; i++){
                memset(ioData->mBuffers[i].mData, 0, ioData->mBuffers[i].mDataByteSize);
//                FLog("音频数据不够(%d)", inBusNumber);
            }
        }
        ctx->bufferMutex.unlock();
    }
    
    return status;
}
