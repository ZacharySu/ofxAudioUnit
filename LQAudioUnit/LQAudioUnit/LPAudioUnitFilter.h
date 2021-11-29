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
@property (nonatomic, weak) id<LPAudioUnitDelegate>delegate;
- (void)testStart;
- (void)testStop;

@end

NS_ASSUME_NONNULL_END
