//
//  LPAudioUnitFilter.m
//  LQAudioUnit
//
//  Created by suzhou on 2021/11/25.
//

#import "LPAudioUnitFilter.h"
#import "ofxAudioUnit.h"

static void audioCallback(void* impl, uint8_t *data, int sampleRate, int channels, int depth, int length);

@interface LPAudioUnitFilter(){
    ofxAudioUnit compressor;
    ofxAudioUnit delay;
    ofxAudioUnit distortion;
    ofxAudioUnit lowPassFilter;
    ofxAudioUnitInput capture;
    
    ofxAudioUnitFilePlayer source1, source2, source3;
    ofxAudioUnitMixer mixer;
    ofxAudioUnitRender render;
    ofxAudioUnitOutput output;
    
    lpAudioUnitSamplesOutput samplesOutput;
    //测试用
    BOOL savepcm;
    BOOL loadpcm;
    FILE* pcmfile;
}
@property (nonatomic, weak) id<LPAudioUnitDelegate>delegate;
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
- (void)testStart {
    savepcm = NO;
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
    
    mixer.setInputBusCount(4);
    source1.connectTo(mixer, 0);
    source2.connectTo(mixer, 1);
    source3.connectTo(mixer, 2);
    capture.connectTo(mixer, 3);
    mixer.setInputVolume(0.5 , 0);
    mixer.setInputVolume(0.05, 1);
    mixer.setInputVolume(0.05, 2);
    
    mixer.connectTo(samplesOutput);
    samplesOutput.setOutputASBD(_sampleRate, _channels, _bitDepth);
    samplesOutput.setOutputSize(2048);
    if(_enableEcho){//-------- hardware device render
        samplesOutput.connectTo(render);
        render.start();
    } else { //-------- VoiceProcessingIO
        samplesOutput.startProcess();
    }

    capture.start();

    source1.loop();
    source2.loop();
    source3.loop();
    _isRunning = YES;
}

- (void)testStop {
    render.stop();
    samplesOutput.stopProcess();
    source1.stop();
    source2.stop();
    source3.stop();
    _isRunning = NO;
}

- (void)setDelegate:(id<LPAudioUnitDelegate>)delegate pcmBytes:(int)pcmBytes{
    NSAssert(!_isRunning, @"LPAudioUnitFilter set delegate in running");
    _delegate = delegate;
    samplesOutput.setOutputCallback((__bridge void*)self, &audioCallback);
    samplesOutput.setOutputSize(pcmBytes);
}

void audioCallback(void* impl, uint8_t *data, int sampleRate, int channels, int depth, int length){
    __strong LPAudioUnitFilter *audioUnit = (__bridge LPAudioUnitFilter*)impl;
    if(audioUnit && audioUnit.delegate && [audioUnit.delegate respondsToSelector:@selector(audioRended:sampleRate:channels:depth:length:)]){
        [audioUnit.delegate audioRended:data sampleRate:sampleRate channels:channels depth:depth length:length];
    }
    if( audioUnit->savepcm && audioUnit->pcmfile != NULL  ){
        fwrite( data, sizeof(uint8_t), length, audioUnit->pcmfile );
    }
//    return NULL;
}

- (void)updateAudioPcmForIndex:(int)index data:(uint8_t *) data length:(int)length{
    
}
@end
