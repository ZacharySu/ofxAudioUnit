//
//  LPAudioUnitFilter.h
//  LQAudioUnit
//
//  Created by suzhou on 2021/11/25.
//

#import <Foundation/Foundation.h>
#import <CoreAudio/CoreAudio.h>

NS_ASSUME_NONNULL_BEGIN

@protocol LPAudioUnitDelegate <NSObject>
-(void)audioRenderd:(uint8_t *) data sampleRate:(int)sampleRate channels:(int)channels depth:(int)depth length:(int)length;
@end

@interface LPAudioUnitFilter : NSObject
@property (nonatomic, assign)   BOOL    enableEcho;

- (void)setDelegate:(id<LPAudioUnitDelegate>)delegate sampleRate:(int)sampleRate channels:(int)channels bitDepth:(int)depth pcmBytes:(int)pcmBytes;

- (void)setMixInputBusCount:(uint8_t)busCount;

- (void)startProcess;
- (void)stopProcess;

- (void)updateAudioPcmForIndex:(int)index data:(void *) data asbd:(AudioStreamBasicDescription) asbd length:(int)length;
@end

NS_ASSUME_NONNULL_END
