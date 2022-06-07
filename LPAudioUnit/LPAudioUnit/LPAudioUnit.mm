//
//  LPAudioUnit.m
//  LPAudioUnit
//
//  Created by suzhou on 2022/5/27.
//

#import <Foundation/Foundation.h>
#import "LPAudioUnit.h"
#include <cstdio>
#include <cstdarg>
#import "ofxAudioUnit.h"

@interface LPAudioUnit()
@property (nonatomic, copy) void(^logBlock)(NSString* log) ;
@end
@implementation LPAudioUnit
+ (LPAudioUnit *)sharedInstance{
    static dispatch_once_t onceToken;
    static LPAudioUnit* instance = nil;
    dispatch_once(&onceToken, ^{
        instance = [[self alloc] init];
    });
    return instance;
}
+ (void)setLogWriterBlock:(void(^)(NSString* log)) block{
    [LPAudioUnit sharedInstance].logBlock = block;
}
- (void)logPrintInternal:(NSString *)logContent{
    if(nil != _logBlock){
        _logBlock([@"[LPAU] " stringByAppendingString:logContent]);
    } else {
        NSLog(@"[LPAU] %@", logContent);
    }
}
- (void)logWithFormat:(NSString*) format, ...{
    va_list paramList;
    va_start(paramList, format);
    NSString* logContent = [[NSString alloc] initWithFormat:format arguments:paramList];
    [self logPrintInternal:logContent];
    va_end(paramList);
}
- (void)logWithFormat:(const char *)format args:(va_list)paramList{
//    va_list paramList;
//    va_start(paramList, format);
    va_list args2;
    va_copy(args2, paramList);
    const int buflen = 1+std::vsnprintf(nullptr, 0, format, paramList);
    char buf[buflen];
//    va_end(paramList);
    int ret = std::vsnprintf(buf, buflen, format, args2);
    va_end(args2);
    
    NSString* logContent = [NSString stringWithCString:(const char*)buf encoding:NSUTF8StringEncoding];
    
    [self logPrintInternal:logContent];
    
}
extern "C" void FLog( const char* fmt, ...){
    va_list paramList;
    va_start(paramList, fmt);
    [[LPAudioUnit sharedInstance] logWithFormat:fmt args:paramList];
    va_end(paramList);
}
extern "C" void DLog( const char* fmt, ...){
    va_list paramList;
    va_start(paramList, fmt);
    NSString* logContent = [[NSString alloc] initWithFormat:[NSString stringWithUTF8String:fmt] arguments:paramList];
    NSLog(@"%@", logContent);
}
@end
