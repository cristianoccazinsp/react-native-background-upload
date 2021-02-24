//  VydiaRNFileUploader.h

#import <Foundation/Foundation.h>
#import <MobileCoreServices/MobileCoreServices.h>
#import <React/RCTEventEmitter.h>
#import <React/RCTBridgeModule.h>

@interface VydiaRNFileUploader : RCTEventEmitter <RCTBridgeModule, NSURLSessionTaskDelegate>
    @property (nonatomic) BOOL isObserving;
    +(VydiaRNFileUploader*)sharedInstance;
    -(void)setBackgroundSessionCompletionHandler:(void (^)(void))handler;
@end
