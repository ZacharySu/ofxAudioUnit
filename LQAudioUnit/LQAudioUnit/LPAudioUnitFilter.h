//
//  LPAudioUnitFilter.h
//  LQAudioUnit
//
//  Created by suzhou on 2021/11/25.
//

#import <Foundation/Foundation.h>

NS_ASSUME_NONNULL_BEGIN

@protocol LPAudioUnitDelegate <NSObject>
-(void)audioRended:(uint8_t *) data sampleRate:(int)sampleRate channels:(int)channels depth:(int)depth length:(int)length;
@end

@interface LPAudioUnitFilter : NSObject
@property (nonatomic, assign)   int     sampleRate;
@property (nonatomic, assign)   int     channels;
@property (nonatomic, assign)   int     bitDepth;
@property (nonatomic, assign)   BOOL    enableEcho;

- (void)testStart;
- (void)testStop;
- (void)setDelegate:(id<LPAudioUnitDelegate>)delegate pcmBytes:(int)pcmBytes;
- (void)updateAudioPcmForIndex:(int)index data:(uint8_t *) data length:(int)length;
@end

NS_ASSUME_NONNULL_END
