//
//  LPAudioUnitFilter.m
//  LPAudioUnit
//
//  Created by suzhou on 2021/11/25.
//

#import "LPAudioUnitFilter.h"
#import "ofxAudioUnit.h"

#define FOR_DEBUG 0
static void audioCallback(void* impl, uint8_t *data, int sampleRate, int channels, int depth, int length);

@interface LPAudioUnitFilter(){
#if FOR_DEBUG
    ofxAudioUnit compressor;
    ofxAudioUnit delay;
    ofxAudioUnit distortion;
    ofxAudioUnit lowPassFilter;
    ofxAudioUnitInput capture;
    
    ofxAudioUnitFilePlayer source1, source2, source3;
    
#endif
    uint8_t destinationBusOffset;
    ofxAudioUnitRawMixer mixer;
    lpAudioUnitSamplesOutput samplesOutput;
    ofxAudioUnitRender *render;
    //测试用
    BOOL savepcm;
    BOOL loadpcm;
    FILE* pcmfile;
}
@property (nonatomic, weak) id<LPAudioUnitDelegate>delegate;

@property (nonatomic, assign)   int     sampleRate;
@property (nonatomic, assign)   int     channels;
@property (nonatomic, assign)   int     bitDepth;

@property (atomic, assign)  BOOL    isRunning;
@end

@implementation LPAudioUnitFilter
- (instancetype)init
{
    self = [super init];
    if (self) {
        _sampleRate = 44100;
        _channels = 2;
        _bitDepth = 16;
        render = NULL;
        destinationBusOffset =  FOR_DEBUG ? 3 : 0;
    }
    return self;
}
+ (NSString *)getFilePath:(NSString* )fileName {
    NSBundle *bundle = [NSBundle bundleWithPath:[[NSBundle mainBundle].resourcePath stringByAppendingPathComponent:@"lovepea.bundle"]];
    NSString *filePath = [bundle pathForResource:fileName ofType:nil];
    if(![[NSFileManager defaultManager] fileExistsAtPath:filePath]){
        NSAssert(NO, @"File(%@) not exist", fileName);
        return nil;
    }
    return filePath;
}

#if FOR_DEBUG
- (void)testStart {
    savepcm = YES;
    if( savepcm ){
        pcmfile = fopen( "/System/Volumes/Data/Users/suzhou/Downloads/sample.pcm","wb");
    }else if(loadpcm){
        pcmfile = fopen( "/tmp/mic.pcm","rb");
    }
    
    source1.setFile([self.class getFilePath:@"audio_long8mulaw.wav"].UTF8String);
    source2.setFile([self.class getFilePath:@"snare.wav"].UTF8String);
    source3.setFile([self.class getFilePath:@"hats.wav"].UTF8String);
    
    distortion.setup(kAudioUnitType_Effect, kAudioUnitSubType_Distortion);
    delay.setup(kAudioUnitType_Effect, kAudioUnitSubType_Delay);
    lowPassFilter.setup(kAudioUnitType_Effect, kAudioUnitSubType_LowPassFilter);
    
    mixer.setInputBusCount(5);
    source1.connectTo(mixer, 0);
    source2.connectTo(mixer, 1);
    source3.connectTo(mixer, 2);
//    capture.connectTo(mixer, 3);
    mixer.setInputVolume(0.5 , 0);
    mixer.setInputVolume(0.05, 1);
    mixer.setInputVolume(0.05, 2);
    
    mixer.setInputVolume(0.6, 4);
    
    mixer.connectTo(samplesOutput);
    if(_enableEcho){//-------- hardware device render
        if(!render){
            render = new ofxAudioUnitRender;
        }
        samplesOutput.connectTo(*render);
        render->start();
    } else { //-------- VoiceProcessingIO
        samplesOutput.startProcess();
    }

//    capture.start();

    source1.loop();
    source2.loop();
    source3.loop();
    _isRunning = YES;
}

- (void)testStop {
    if(render){
        render->stop();
        delete render;
    }
    samplesOutput.stopProcess();
    source1.stop();
    source2.stop();
    source3.stop();
    _isRunning = NO;
}

#endif

- (void)setEnableEcho:(BOOL)enableEcho{
    NSAssert(!_isRunning, @"LPAudioUnitFilter set setEnableEcho in running");
    _enableEcho = enableEcho;
}
- (void)setMixInputBusCount:(uint8_t)busCount{
    NSAssert(!_isRunning, @"LPAudioUnitFilter set setMixInputBusCount in running");
    mixer.setInputBusCount(busCount + destinationBusOffset);
}

- (void)startProcess{
#if FOR_DEBUG
    [self testStart];
#else
    mixer.connectTo(samplesOutput);
    if(_enableEcho){//-------- hardware device render
        if(!render){
            render = new ofxAudioUnitRender;
        }
        samplesOutput.connectTo(*render);
        render->start();
    } else { //-------- VoiceProcessingIO
        samplesOutput.startProcess();
    }
    _isRunning = YES;
#endif
}
- (void)stopProcess{
#if FOR_DEBUG
    [self testStop];
#else
    if(render){
        render->stop();
        delete render;
        render = NULL;
    }
    samplesOutput.stopProcess();
    _isRunning = NO;
#endif
}

- (void)setDelegate:(id<LPAudioUnitDelegate>)delegate sampleRate:(int)sampleRate channels:(int)channels bitDepth:(int)depth pcmBytes:(int)pcmBytes{
    NSAssert(!_isRunning, @"LPAudioUnitFilter set delegate in running");
    _sampleRate = sampleRate;
    _channels = channels;
    _bitDepth = depth;
    _delegate = delegate;
    samplesOutput.setOutputSize(pcmBytes);
    samplesOutput.setOutputASBD(_sampleRate, _channels, _bitDepth);
    samplesOutput.setOutputCallback((__bridge void*)self, &audioCallback);
    samplesOutput.setOutputSize(pcmBytes);
}

void audioCallback(void* impl, uint8_t *data, int sampleRate, int channels, int depth, int length){
    __strong LPAudioUnitFilter *audioUnit = (__bridge LPAudioUnitFilter*)impl;
    if(audioUnit && audioUnit.delegate && [audioUnit.delegate respondsToSelector:@selector(audioRenderd:sampleRate:channels:depth:length:)]){
        [audioUnit.delegate audioRenderd:data sampleRate:sampleRate channels:channels depth:depth length:length];
    }
//    if( audioUnit->savepcm && audioUnit->pcmfile != NULL  ){
//        fwrite( data, sizeof(uint8_t), length, audioUnit->pcmfile );
//    }
//    return NULL;
}
- (void)updateAudioPcmForIndex:(int)index data:(void *) data asbd:(AudioStreamBasicDescription) asbd length:(int)length{
    if(!_isRunning){
        return;
    }
    mixer.updateAudioPcmBuffer(asbd, data, length, index + destinationBusOffset);
        if( savepcm && pcmfile != NULL  ){
            size_t sizeWriten = fwrite( data, sizeof(uint8_t), length, pcmfile );
            if(sizeWriten <= 0){
                printf("fail to write\n");
            }
        }
}
@end
