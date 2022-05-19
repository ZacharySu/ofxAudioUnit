//
//  LPAudioUnitFft.m
//  LPAudioUnit
//
//  Created by suzhou on 2022/5/18.
//

#import "LPAudioUnitFft.h"
#import "ofxAudioUnit.h"
#include "ofxAudioUnitHardwareUtils.h"

#define FOR_DEBUG 0
@interface LPAudioUnitFft(){
    ofxAudioUnitInput   audioCaputure;
    ofxAudioUnitRender render;
    ofxAudioUnitFftNode fftNode;
    ofxAudioUnitRawMixer mixer;
    ofxAudioUnitFilePlayer source;
}
@property (nonatomic, strong) NSTimer *fftTimer;
@end

@implementation LPAudioUnitFft
- (instancetype)init
{
    self = [super init];
    if (self) {
        
    }
    return self;
}
+ (AudioDeviceID)getDeivceId:(NSString *)uniqueID{
    return GetAudioDeviceID(std::string(uniqueID.UTF8String));
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
- (void)startProcess{
    source.setFile([self.class getFilePath:@"hats.wav"].UTF8String);
    source.connectTo(mixer).connectTo(render);
    source.loop();
    render.start();
}
#else
- (void)startProcess{

    fftNode.setScale(OFXAU_SCALE_DECIBEL);
    if(_isCapturer){
        audioCaputure.listInputDevices();
        if(self.uniqueID){
            audioCaputure.setDevice([LPAudioUnitFft getDeivceId:self.uniqueID]);
        }
        mixer.setOutputVolume(0);
        audioCaputure.connectTo(fftNode).connectTo(mixer).connectTo(render);
        render.start();
        audioCaputure.start();
    }else {
        if(self.uniqueID){
            render.setDevice([LPAudioUnitFft getDeivceId:self.uniqueID]);
        }
        source.setFile([self.class getFilePath:@"snare.wav"].UTF8String);
        source.connectTo(mixer).connectTo(fftNode).connectTo(render);
        source.loop();
        render.start();
    }
    self.fftTimer =  [NSTimer timerWithTimeInterval:1.0/50
                                                      target:self selector:@selector(fftTimerFire:) userInfo:nil repeats:YES];
      [NSRunLoop.mainRunLoop addTimer:self.fftTimer forMode:NSRunLoopCommonModes];

}
#endif
- (void)stopProcess{
    audioCaputure.stop();
    render.stop();
    if(self.fftTimer){
        [self.fftTimer invalidate];
        self.fftTimer = nil;
    }
}
- (void)fftTimerFire:(NSTimer *)timer{
    if([timer isEqualTo:self.fftTimer]){
        float soundDB = -50.0;
#if 0
        std::vector<float> outPhase;
//        printf("\n%lu[", outPhase.size());
        fftNode.getAmplitude(outPhase);
        auto phase = std::max_element(outPhase.begin(), outPhase.end());
        if( phase != outPhase.end()){
            printf("%f, ",  *phase);
        }
        //        printf("]\n");
#else
        soundDB = fftNode.getdBLevel();
//        printf("%f dB/",  soundDB);
#endif

        if(self.delegate && [self.delegate respondsToSelector:@selector(onSoundLevelUpdate:)]){
            [self.delegate onSoundLevelUpdate:@(soundDB)];
        }
    }
    
}
@end
