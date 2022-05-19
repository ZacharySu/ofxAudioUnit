//
//  LPAudioUnitFft.h
//  LPAudioUnit
//
//  Created by suzhou on 2022/5/18.
//

#import <Foundation/Foundation.h>

NS_ASSUME_NONNULL_BEGIN

@protocol LPAudioUnitFftDelegate <NSObject>

/*
 soundDB float (-50.0 ~ 0)
 */
- (void)onSoundLevelUpdate:(NSNumber *)level;

@end
@interface LPAudioUnitFft : NSObject
@property (nonatomic, copy) NSString    *uniqueID;
@property (nonatomic, assign)   BOOL    isCapturer;//YES:capture; NO:render
@property (nonatomic, weak)     id<LPAudioUnitFftDelegate> delegate;
- (void)startProcess;
- (void)stopProcess;
@end

NS_ASSUME_NONNULL_END
