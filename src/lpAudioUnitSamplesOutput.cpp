//
//  lpAudioUnitSamplesOutput.cpp
//  LPAudioUnit
//
//  Created by suzhou on 2021/11/29.
//

#include "lpAudioUnitSamplesOutput.hpp"
#include "ofxAudioUnitUtils.h"
#include "TPCircularBuffer.h"

using namespace std;
// a passthru render callback which copies the rendered samples in the process
static OSStatus RenderAndCopy(void * inRefCon,
                              AudioUnitRenderActionFlags *    ioActionFlags,
                              const AudioTimeStamp *    inTimeStamp,
                              UInt32 inBusNumber,
                              UInt32    inNumberFrames,
                              AudioBufferList * ioData);
static OSStatus CallBack(
                            void      *inRefCon,
                            AudioUnitRenderActionFlags  *ioActionFlags,
                            const AudioTimeStamp   *inTimeStamp,
                            UInt32       inBusNumber,
                            UInt32       inNumberFrames,
                         AudioBufferList    *ioData);
struct PCMOutputContext{
    TPCircularBuffer circularBuffer;
    std::mutex bufferMutex;
    void* outputImpl;
    void (*outputCallback)(void* impl, uint8_t *data, int sampleRate, int channels, int depth, int length);
    int outputSampleRate;
    int outputChannels;
    int outputBitDepth;
    unsigned int requireOutputSize;
    
    PCMOutputContext()
    :_bufferSize(0),
    outputCallback(NULL),
    outputImpl(NULL),
    requireOutputSize(0)
    {}
    void setCircularBufferSize(unsigned int size){
        if(_bufferSize != size){
            bufferMutex.lock();
            TPCircularBufferCleanup(&circularBuffer);
            TPCircularBufferInit(&circularBuffer, size);
            _bufferSize = size;
            bufferMutex.unlock();
        }
    }
private:
    unsigned int _bufferSize;
};
struct lpAudioUnitSamplesOutput::PCMOutputImpl{
    PCMOutputContext ctx;
};

lpAudioUnitSamplesOutput::lpAudioUnitSamplesOutput()
:_impl(std::make_shared<PCMOutputImpl>() )
{
    setup(kAudioUnitType_FormatConverter, kAudioUnitSubType_AUConverter);
    outputIsRunning = false;
    
    
    UInt32 size = sizeof(outputASBD);
    // set output stream format and callback for mixer
    AudioUnitGetProperty(*_unit, kAudioUnitProperty_StreamFormat, kAudioUnitScope_Output, 0, &outputASBD, &size);
    _impl->ctx.outputSampleRate = outputASBD.mSampleRate;
    _impl->ctx.outputChannels = outputASBD.mChannelsPerFrame;
    _impl->ctx.outputBitDepth = outputASBD.mBitsPerChannel;

}
lpAudioUnitSamplesOutput::~lpAudioUnitSamplesOutput(){
    TPCircularBufferCleanup(&_impl->ctx.circularBuffer);
}
    
// ----------------------------------------------------------
void lpAudioUnitSamplesOutput::setOutputASBD(int sampleRate, int channels, int bitDepth)
// ----------------------------------------------------------
{
    _impl->ctx.outputSampleRate = sampleRate;
    _impl->ctx.outputChannels = channels;
    _impl->ctx.outputBitDepth = bitDepth;
    
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
    _impl->ctx.outputImpl = impl;
    _impl->ctx.outputCallback = callback;
    OFXAU_PRINT(AudioUnitAddRenderNotify(*_unit,
                                         &RenderAndCopy, &_impl->ctx),
                "Mixer setOutputCallback");
}

void lpAudioUnitSamplesOutput::setOutputSize(unsigned int size){
    _impl->ctx.requireOutputSize = size;
    _impl->ctx.setCircularBufferSize(size*1.2);
}
bool lpAudioUnitSamplesOutput:: startProcess(){
    OFXAU_RET_FALSE(NewAUGraph(&auGraph), "NewAUGraph failed");
    AudioComponentDescription componentDesc = {
        .componentType = kAudioUnitType_Output,
#if !TARGET_OS_IPHONE
        .componentSubType = kAudioUnitSubType_VoiceProcessingIO,
#else
        .componentSubType = kAudioUnitSubType_RemoteIO,1
#endif
        .componentManufacturer = kAudioUnitManufacturer_Apple
    };
    AUNode remoteIONode;
    OFXAU_RET_FALSE(AUGraphOpen(auGraph),"couldn't AUGraphOpen"); //打开AUGraph
    OFXAU_RET_FALSE(AUGraphAddNode(auGraph,&componentDesc,&remoteIONode),"couldn't add remote io node");
    AudioUnit remoteIOUnit;
    OFXAU_RET_FALSE(AUGraphNodeInfo(auGraph,remoteIONode,NULL,&remoteIOUnit),"couldn't get remote io unit from node");
    
    AudioStreamBasicDescription mAudioFormat;
    UInt32 ASBDSize = sizeof(mAudioFormat);
    OFXAU_PRINT(AudioUnitGetProperty(*_unit,
                                     kAudioUnitProperty_StreamFormat,
                                     kAudioUnitScope_Output,
                                     0,
                                     &mAudioFormat,
                                     &ASBDSize),
                "getting self au output format");
        
    OFXAU_RET_FALSE(AudioUnitSetProperty(remoteIOUnit,
                                        kAudioUnitProperty_StreamFormat,
                                        kAudioUnitScope_Input,
                                        0,
                                        &mAudioFormat,
                                        sizeof(mAudioFormat)),"couldn't set kAudioUnitProperty_StreamFormat with kAudioUnitScope_Input");
    
    AudioUnitConnection connection;
    connection.sourceAudioUnit    = *_unit;
    connection.sourceOutputNumber = 0;
    connection.destInputNumber    = 0;
    
    OFXAU_PRINT(AudioUnitSetProperty(remoteIOUnit,
                                     kAudioUnitProperty_MakeConnection,
                                     kAudioUnitScope_Input,
                                     0,
                                     &connection,
                                     sizeof(AudioUnitConnection)),
                "connecting units");
    
    AURenderCallbackStruct inputProc;
    inputProc.inputProc = CallBack;
    inputProc.inputProcRefCon = this;
    OFXAU_RET_FALSE(AUGraphSetNodeInputCallback(auGraph, remoteIONode, 0, &inputProc),"Error setting io output callback");
     
    OFXAU_RET_FALSE(AUGraphInitialize(auGraph),"couldn't AUGraphInitialize" );
    OFXAU_RET_FALSE(AUGraphUpdate(auGraph, NULL),"couldn't AUGraphUpdate" );

    //最后再调用以下代码即可开始录音
    OFXAU_RET_FALSE(AUGraphStart(auGraph),"couldn't AUGraphStart");
    CAShow(auGraph);
    OFXAU_RET_FALSE(AudioUnitInitialize(remoteIOUnit), "initializing unit");
    OFXAU_RET_FALSE(AudioOutputUnitStart(remoteIOUnit), "starting output unit");
    outputIsRunning = true;
    return true;
}
void lpAudioUnitSamplesOutput:: stopProcess(){
    if (outputIsRunning) {
        Boolean isRunning = false;
        
        OFXAU_RETURN(AUGraphIsRunning(auGraph, &isRunning), "AUGraphIsRunning failed");
        if (isRunning) {
            OFXAU_RETURN(AUGraphStop(auGraph), "AUGraphStop failed");
        }
    }
    outputIsRunning = false;
}
// ----------------------------------------------------------
OSStatus RenderAndCopy(void * inRefCon,
                       AudioUnitRenderActionFlags * ioActionFlags,
                       const AudioTimeStamp * inTimeStamp,
                       UInt32 inBusNumber,
                       UInt32 inNumberFrames,
                       AudioBufferList * ioData)
{
    OSStatus status = 0;
    
    if ((*ioActionFlags & kAudioUnitRenderAction_PostRender) && inBusNumber == 0)
    {
        PCMOutputContext* ctx = (PCMOutputContext*)inRefCon;
        for (int bufferIndex = 0; bufferIndex < ioData->mNumberBuffers; bufferIndex++) {
            unsigned char *audioBufferPtr = (unsigned char *)ioData->mBuffers[bufferIndex].mData;
            int dataLeftSize = (int)ioData->mBuffers[bufferIndex].mDataByteSize;
//            cout << "push " << bufferIndex << ":" << dataLeftSize << endl;
            if(ctx->outputCallback){
                if(ctx->requireOutputSize > 0 && ctx->requireOutputSize != dataLeftSize){
                    while (dataLeftSize > 0) {
                        int32_t circBufferSize;
                        unsigned char * circBufferTail = (unsigned char *)TPCircularBufferTail(&ctx->circularBuffer, &circBufferSize);
                        if(circBufferSize > 0){//有残留数据未发送
                            uint32_t sizeToSave = (uint32_t)std::min<size_t>(dataLeftSize, ctx->requireOutputSize - circBufferSize);
                            sizeToSave = TPCircularBufferAddData(&ctx->circularBuffer, audioBufferPtr, sizeToSave);
//                            cout << "save + " << sizeToSave << endl;
                            dataLeftSize -= sizeToSave;
                            audioBufferPtr += sizeToSave;
                            circBufferTail = (unsigned char *)TPCircularBufferTail(&ctx->circularBuffer, &circBufferSize);
                            if(circBufferSize == ctx->requireOutputSize){ //数据足够输出需求
//                                cout << "save + " << sizeToSave << endl;
                                ctx->outputCallback(ctx->outputImpl, circBufferTail, ctx->outputSampleRate, ctx->outputChannels, ctx->outputBitDepth, ctx->requireOutputSize);
                                TPCircularBufferConsume(&ctx->circularBuffer, ctx->requireOutputSize);
                            }
                        } else {//没有残留数据
                            if(dataLeftSize >= ctx->requireOutputSize){//足够发送一次
                                ctx->outputCallback(ctx->outputImpl, audioBufferPtr, ctx->outputSampleRate, ctx->outputChannels, ctx->outputBitDepth, ctx->requireOutputSize);
                                dataLeftSize -= ctx->requireOutputSize;
                                audioBufferPtr += ctx->requireOutputSize;
                            } else {//数据不够发送
                                TPCircularBufferAddData(&ctx->circularBuffer, audioBufferPtr, dataLeftSize);
                                dataLeftSize -= dataLeftSize;
                                audioBufferPtr += dataLeftSize;
                            }
                        }
                    }
                } else {
                    ctx->outputCallback(ctx->outputImpl, audioBufferPtr, ctx->outputSampleRate, ctx->outputChannels, ctx->outputBitDepth, dataLeftSize);
                }
            }
        }
    }
    
    return status;
}

static OSStatus CallBack(
                            void      *inRefCon,
                            AudioUnitRenderActionFlags  *ioActionFlags,
                            const AudioTimeStamp   *inTimeStamp,
                            UInt32       inBusNumber,
                            UInt32       inNumberFrames,
                            AudioBufferList    *ioData)
{
    lpAudioUnitSamplesOutput * sourceUnit = static_cast<lpAudioUnitSamplesOutput *>(inRefCon);
    if(sourceUnit->getUnitRef()){
        sourceUnit->render(ioActionFlags, inTimeStamp, inBusNumber, inNumberFrames, ioData);
    }
    for(int i = 0; i < ioData->mNumberBuffers; i++) {
        memset(ioData->mBuffers[i].mData, 0, ioData->mBuffers[0].mDataByteSize);
    }
    *ioActionFlags |= kAudioUnitRenderAction_OutputIsSilence;
    
    return noErr;//renderErr;
}
