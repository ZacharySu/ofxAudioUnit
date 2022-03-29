#include "ofxAudioUnitRawMixer.h"
#include "ofxAudioUnitUtils.h"
#include "TPCircularBuffer.h"

#ifndef MAX
#define MAX(x,y) ((x)>(y) ?(x):(y))
#endif

using namespace std;

struct RawMixerConext{
    std::vector<AudioStreamBasicDescription> asbds;
    
    std::vector<TPCircularBuffer> circularBuffers;
    std::mutex bufferMutex;
    
    RawMixerConext()
    : _bufferSize(0)
    {}
    UInt32 getRawInputSize(){
        return _bufferSize;
    }
    void setRawInputSize(UInt32 bufferCount, UInt32 samplesToBuffer){
        if(bufferCount != circularBuffers.size() || samplesToBuffer != _bufferSize) {
            bufferMutex.lock();
            {
                for(int i = 0; i < circularBuffers.size(); i++) {
                    TPCircularBufferCleanup(&circularBuffers[i]);
                }
                
                circularBuffers.resize(bufferCount);
                
                for(int i = 0; i < circularBuffers.size(); i++) {
                    TPCircularBufferInit(&circularBuffers[i], samplesToBuffer * sizeof(Float32));
                }
                _bufferSize = samplesToBuffer;
            }
            bufferMutex.unlock();
        }
    }
    bool asbdIsChange(AudioStreamBasicDescription asbd, uint32_t destinationBus){
        if(destinationBus >= asbds.size()){
            asbds.resize(destinationBus+1);
        }
        bufferMutex.lock();
        if(memcmp(&asbd, &asbds[destinationBus], sizeof(AudioStreamBasicDescription))){
            memcpy(&asbds[destinationBus], &asbd, sizeof(AudioStreamBasicDescription));
            if(destinationBus < circularBuffers.size()){
                TPCircularBufferClear(&circularBuffers[destinationBus]);
            }
            cout << "ASBD " << destinationBus << " change" << endl;
            bufferMutex.unlock();
            return true;
        }
        bufferMutex.unlock();
        return false;
    }
private:
    UInt32 _bufferSize;
};
static OSStatus RenderAndCopy(void * inRefCon,
                              AudioUnitRenderActionFlags *    ioActionFlags,
                              const AudioTimeStamp *    inTimeStamp,
                              UInt32 inBusNumber,
                              UInt32    inNumberFrames,
                              AudioBufferList * ioData);


struct ofxAudioUnitRawMixer::RawMixerImpl{
    RawMixerConext ctx;
};

// ----------------------------------------------------------
ofxAudioUnitRawMixer::ofxAudioUnitRawMixer()
:_impl(make_shared<RawMixerImpl>() )
// ----------------------------------------------------------
{
    
}

ofxAudioUnitRawMixer::~ofxAudioUnitRawMixer(){
    for(int i = 0; i < _impl->ctx.circularBuffers.size(); i++) {
        TPCircularBufferCleanup(&_impl->ctx.circularBuffers[i]);
    }
}
bool ofxAudioUnitRawMixer::setInputBusCount(unsigned int numberOfInputBusses, unsigned int samplesToBuffer){
    ofxAudioUnitMixer::setInputBusCount(numberOfInputBusses);
    _impl->ctx.setRawInputSize(numberOfInputBusses, samplesToBuffer);
    _impl->ctx.asbds.resize(numberOfInputBusses);
    return true;
}
void ofxAudioUnitRawMixer::setInputStreamFormat(AudioStreamBasicDescription ASBD, uint32_t destinationBus){
    OFXAU_RETURN(AudioUnitSetProperty(*_unit,
                                 kAudioUnitProperty_StreamFormat,
                                 kAudioUnitScope_Input,
                                 destinationBus,
                                 &ASBD,
                                 sizeof(ASBD)),
            "setting hardware input destination's format");
    AURenderCallbackStruct callback = {RenderAndCopy, &_impl->ctx};
    setRenderCallback(callback, destinationBus);
}
void ofxAudioUnitRawMixer::setInputStreamFormat(int sampleRate, int channels, int bitDepth, uint32_t destinationBus)
{
    AudioStreamBasicDescription ASBD;
    UInt32 ASBDSize = sizeof(ASBD);
    OFXAU_RETURN(AudioUnitGetProperty(*_unit,
                                     kAudioUnitProperty_StreamFormat,
                                     kAudioUnitScope_Input,
                                     destinationBus,
                                     &ASBD,
                                     &ASBDSize),
                "getting hardware input's output format");
    ASBD.mFormatID            = kAudioFormatLinearPCM;
    ASBD.mFormatFlags        =  kAudioFormatFlagIsSignedInteger |kAudioFormatFlagIsPacked|kAudioFormatFlagIsNonInterleaved;
    ASBD.mSampleRate = sampleRate;
    ASBD.mFramesPerPacket    = 1;
    ASBD.mChannelsPerFrame = channels;
    ASBD.mBitsPerChannel = bitDepth;
    ASBD.mBytesPerFrame = ASBD.mBitsPerChannel*ASBD.mChannelsPerFrame/8;//每帧的bytes数
    ASBD.mBytesPerPacket = ASBD.mBytesPerFrame*ASBD.mFramesPerPacket;
    OFXAU_RETURN(AudioUnitSetProperty(*_unit,
                                 kAudioUnitProperty_StreamFormat,
                                 kAudioUnitScope_Input,
                                 destinationBus,
                                 &ASBD,
                                 sizeof(ASBD)),
            "setting hardware input destination's format");
    AURenderCallbackStruct callback = {RenderAndCopy, &_impl->ctx};
    setRenderCallback(callback, destinationBus);
}
void ofxAudioUnitRawMixer::updateAudioPcmBuffer(void *data, int byteSize, uint32_t destinationBus){
    if(destinationBus < _impl->ctx.circularBuffers.size()){
        if(_impl->ctx.getRawInputSize() < byteSize){
            _impl->ctx.setRawInputSize((UInt32)_impl->ctx.circularBuffers.size(), byteSize);
        }
        if(_impl->ctx.bufferMutex.try_lock()) {
            int32_t availableBytesInCircBuffer;
            TPCircularBufferHead(&_impl->ctx.circularBuffers[destinationBus], &availableBytesInCircBuffer);
            
            if(availableBytesInCircBuffer < byteSize) {
                TPCircularBufferConsume(&_impl->ctx.circularBuffers[destinationBus], byteSize - availableBytesInCircBuffer);
                cout << "音频数据[" << destinationBus << "]超出，将删除：" << (byteSize - availableBytesInCircBuffer) << endl;
            }
            
            TPCircularBufferProduceBytes(&_impl->ctx.circularBuffers[destinationBus], data, byteSize);
            _impl->ctx.bufferMutex.unlock();
        }
    } else {
        _impl->ctx.setRawInputSize((UInt32)destinationBus+1, MAX(_impl->ctx.getRawInputSize(), byteSize));
    }
}
void ofxAudioUnitRawMixer::updateAudioPcmBuffer(AudioStreamBasicDescription ASBD, void *data, int byteSize, uint32_t destinationBus){
    if(_impl->ctx.asbdIsChange(ASBD, destinationBus)){
        setInputStreamFormat(ASBD, destinationBus);
    }
    updateAudioPcmBuffer(data, byteSize, destinationBus);
}
OSStatus RenderAndCopy(void * inRefCon,
                       AudioUnitRenderActionFlags * ioActionFlags,
                       const AudioTimeStamp * inTimeStamp,
                       UInt32 inBusNumber,
                       UInt32 inNumberFrames,
                       AudioBufferList * ioData)
{
    OSStatus status = noErr;
//    cout << "render for input bus:" << inBusNumber << endl;
    RawMixerConext * ctx = static_cast<RawMixerConext *>(inRefCon);

    if(ctx->bufferMutex.try_lock()) {
        int32_t availableBytes = 0;
        UInt32 sizeInNeed = 0;
        if(ctx->circularBuffers.size() > inBusNumber){
            TPCircularBufferTail(&ctx->circularBuffers[inBusNumber], &availableBytes);
            
            for(int i = 0; i < ioData->mNumberBuffers; i++){
                sizeInNeed += ioData->mBuffers[i].mDataByteSize;
            }
        }
        if(sizeInNeed > 0 && availableBytes >= sizeInNeed){
            for(int i = 0; i < ioData->mNumberBuffers; i++){
                int bufferToConsume = TPCircularBufferGetData(&ctx->circularBuffers[inBusNumber], ioData->mBuffers[i].mData, ioData->mBuffers[i].mDataByteSize);
//                cout << inBusNumber << "消费了音频数据：" << bufferToConsume << endl;
            }
        } else {
            for(int i = 0; i < ioData->mNumberBuffers; i++){
                memset(ioData->mBuffers[i].mData, 0, ioData->mBuffers[i].mDataByteSize);
//                cout << "音频数据不够(" << inBusNumber << ")" << endl;
            }
        }
        ctx->bufferMutex.unlock();
    }
    
    return status;
}
