//
//  LPAudioUnit.h
//  LPAudioUnit
//
//  Created by suzhou on 2021/11/24.
//

#import <Foundation/Foundation.h>

//! Project version number for LPAudioUnit.
FOUNDATION_EXPORT double LPAudioUnitVersionNumber;

//! Project version string for LPAudioUnit.
FOUNDATION_EXPORT const unsigned char LPAudioUnitVersionString[];

// In this header, you should import all the public headers of your framework using statements like #import <LPAudioUnit/PublicHeader.h>

#import "LPAudioUnitFilter.h"
#import "LPAudioUnitFft.h"

@interface LPAudioUnit: NSObject
+ (void)setLogWriterBlock:(void(^)(NSString* log)) block;
@end
