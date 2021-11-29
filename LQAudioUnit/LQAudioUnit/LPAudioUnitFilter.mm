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
    ofxAudioUnit filter;
    
    ofxAudioUnitFilePlayer source1, source2, source3;
    ofxAudioUnitMixer mixer;
    ofxAudioUnitOutput output;
    
    
    lpAudioUnitSamplesOutput samplesOutput;
    //测试用
    BOOL savepcm;
    BOOL loadpcm;
    FILE* pcmfile;
}

@end

@implementation LPAudioUnitFilter

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
    filter.setup(kAudioUnitType_Effect, kAudioUnitSubType_LowPassFilter);
    
    if(0){
        source1.connectTo(distortion);
        source2.connectTo(delay);
        source3.connectTo(filter);
        
        mixer.setInputBusCount(3);
        
        distortion.connectTo(mixer, 0);
        delay.connectTo(mixer, 1);
        filter.connectTo(mixer, 2);
        //    mixer.setOutputASBD(44100, 2, 16);
        
        //
    } else {
        source1.connectTo(mixer, 0);
        source2.connectTo(mixer, 1);
        source3.connectTo(mixer, 2);
    }
    mixer.setInputVolume(0.05, 1);
    mixer.setInputVolume(0.05, 2);
    if(0){
        mixer.connectTo(compressor);
        compressor.setup(kAudioUnitType_Effect, kAudioUnitSubType_DynamicsProcessor);
        ofxAudioUnitDSPNode dspNode;
        compressor.connectTo(dspNode).connectTo(output);
    } else if(1){
//        samplesOutput.setup(kAudioUnitType_FormatConverter, kAudioUnitSubType_AUConverter);
        mixer.connectTo(samplesOutput);
        samplesOutput.setOutputASBD(44100, 2, 32);
        samplesOutput.connectTo(output);
    } else {
        compressor.setup(kAudioUnitType_Effect, kAudioUnitSubType_DynamicsProcessor);
        mixer.connectTo(compressor);
        compressor.connectTo(output);
    }
//    compressor.connectTo(output);
//    mixer.setInputVolume(0.5, 2);
     
    output.start();

    source1.loop();
    source2.loop();
    source3.loop();
}

- (void)testStop {
    output.stop();

    source1.stop();
    source2.stop();
    source3.stop();
}

- (void)setDelegate:(id<LPAudioUnitDelegate>)delegate{
    _delegate = delegate;
    samplesOutput.setOutputCallback((__bridge void*)self, &audioCallback);
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
@end
