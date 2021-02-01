# Modified code from https://github.com/Vydia/react-native-background-upload.

iOS only background uploader which supports network requests to continue even when the app goes to background. Supports both raw and multipart uploads (file size limits might apply).
In addition, provides methods to request more background time to the OS in order to continue running for longer periods of time. Alternatively,the app can just be waken up after the upload is done. Note that if both features are combined, background time/wake ups will be reduced.


# Installation

## 1. Install package

Add to packages.json: "react-native-background-upload": "github:cristianoccazinsp/react-native-background-upload"


## 2. Link Native Code

### Automatic Native Library Linking

NO LONGER NEEDED. RN 0.60 will auto link. Header import is still needed if we want to listen to events
`react-native link react-native-background-upload`


# Usage

```js
import Upload from 'react-native-background-upload'

const options = {
  url: 'https://myservice.com/path/to/post',
  path: 'file://path/to/file/on/device',
  method: 'POST',
  type: 'raw',
  headers: {
    'content-type': 'application/octet-stream', // Customize content-type
    'my-custom-header': 's3headervalueorwhateveryouneed'
  }
}

Upload.startUpload(options).then((uploadId) => {
  console.log('Upload started')
  Upload.addListener('progress', uploadId, (data) => {
    console.log(`Progress: ${data.progress}%`)
  })
  Upload.addListener('error', uploadId, (data) => {
    console.log(`Error: ${data.error}%`)
  })
  Upload.addListener('cancelled', uploadId, (data) => {
    console.log(`Cancelled!`)
  })
  Upload.addListener('completed', uploadId, (data) => {
    // data includes responseCode: number, responseBody: Object, responseHeaders: Lower cased http headers
    console.log('Completed!')
  })
}).catch((err) => {
  console.log('Upload error!', err)
})
```

## Multipart Uploads - Complete example with cleanup

Just set the `type` option to `multipart` and set the `field` option.  Example:

```js
import RNBackgroundUpload from 'react-native-background-upload';

const resolveUpload = function(uploadId){
  return new Promise((resolve, reject) => {
    let l1 = RNBackgroundUpload.addListener('error', uploadId, (data) => {
      reject(data);
      cleanup();
    });
    let l2 = RNBackgroundUpload.addListener('cancelled', uploadId, (data) => {
      reject(data);
      cleanup();
    });
    let l3 = RNBackgroundUpload.addListener('completed', uploadId, (data) => {
      resolve(data);
      cleanup();
    });
    let cleanup = () => {
      l1.remove(); l2.remove(); l3.remove();
    }
  });
}

const headers = {
  'Authorization': authString,
  'Content-Type': 'multipart/form-data'
}

const uploadUrl = 'some url';
const uri = 'file://path/to/file';
const formData = {
  someKey: 'someValue'
};

const options = {
  url: uploadUrl,
  path: uri || undefined,
  method: 'POST',
  field: 'fileData',
  headers: headers,
  type: 'multipart',
  parameters: formData,
  customUploadId: `u-${new Date().getTime()}`
}

let res;
let responseData = null;

try{
  await RNBackgroundUpload.startUpload(options);
  res = await resolveUpload(options.customUploadId);

}
catch(err){
  throw {
    status: -1,
    _error: err,
    data: {
      'code':null,
      'detail': null,
      'message': "Network error"
    }
  };
}

responseData = res.responseBody;


if(res.responseCode >= 400){
  throw {
    _error: res,
    status: res.responseCode,
    data: api.processError(responseData)
  };
}
else{
  return {
    data: responseData,
    status: res.responseCode,
    _response: res
  }
}
```

Note the `field` property is required for multipart uploads.

# API
TODO

# canSuspendIfBackground()

If you are not using [iOS background events](#ios-background-events), you can ignore this method.

Notify the OS that your app can sleep again. Call this method when your app has done all its work or is waiting for background uploads to complete. Upon calling the method, you app is suspended if it's running in the background. Native code and JS will pause execution. Apple recommends you keep background execution time at less than 5 to 10 sec.

Here are a few common situations and how to handle them:

 - Uploads are finished (completed, error or cancelled) and your app does not need to do any more work. You should call `canSuspendIfBackground` after receiving the events.

 - Uploads are finished (completed, error or cancelled) and your app needs to run some computation or make a network request. You should call `canSuspendIfBackground` after the computation or network call is done.

 - Uploads are finished (completed, error or cancelled) and your app needs to upload some more. You call `startUpload` a number of times and add your listeners. You should call `canSuspendIfBackground` after the uploads start but not wait for them to finish. You also need to call `canSuspendIfBackground` after you have received the events, even if some uploads are cancelled or fail:

```javascript
import { addListener, startUpload, canSuspendIfBackground } from 'react-native-background-upload';

function listenForUploadCompletion(uploadId) {
  return new Promise((resolve, reject) => {
    addListener('error', uploadId, reject);
    addListener('cancelled', uploadId, () => reject(new Error('upload cancelled')));
    addListener('completed', uploadId, ({ responseCode, responseBody }) => {
      if (200 <= responseCode && responseCode <= 299) {
          resolve(uploadId);
      } else {
          reject(new Error(`Could not upload file (${responseCode}):\n${responseBody}`));
      }
    });
  });
}

async function uploadFilesWhileInBackground(url, files) {
  const uploadIds = await Promise.all(files.map(path => startUpload({ path, url })));
  const didUploadPromise = Promise.all(uploadIds.map(id => listenForUploadCompletion(id)));
  // suspend after event listeners are added
  canSuspendIfBackground();
  try {
    await didUploadPromise;
    // update the app UI
  } catch (e) {
    // handle error (show alert, present local notification, etc)
  }
  canSuspendIfBackground();
}
```


# iOS Background Events

By default, iOS does not wake up your app when uploads are done while your app is not in the foreground. To receive the upload events (`error`, `completed`...) while your app is in the background, add the following to your `AppDelegate.m`:

```objective-c
#import <VydiaRNFileUploader.h>

// required for background uploads
- (void)application:(UIApplication *)application
handleEventsForBackgroundURLSession:(NSString *)identifier
  completionHandler:(void (^)(void))completionHandler {
  [[VydiaRNFileUploader sharedInstance] setBackgroundSessionCompletionHandler:completionHandler];
}
```

This means you can do extra work in the background, like make network calls or uploads more files! You _must_ call `canSuspendIfBackground` when you are done processing the events to sleep again. You can safely call this method when you are not in the background.

Here is a JS example:

```javascript
import RNBackgroundUpload from 'react-native-background-upload';

async function uploadFile(url, fileURI) {
  const uploadId = await RNBackgroundUpload.startUpload({ url, path: fileURI, method: 'POST' });
  return new Promise((resolve, reject) => {
    RNBackgroundUpload.addListener('error', uploadId, reject);
    RNBackgroundUpload.addListener('cancelled', uploadId, () => reject(new Error('upload cancelled')));
    RNBackgroundUpload.addListener('completed', uploadId, ({ responseCode, responseBody }) => {
      if (200 <= responseCode && responseCode <= 299) {
          resolve(uploadId);
      } else {
          reject(new Error(`Could not upload file (${responseCode}):\n${responseBody}`));
      }
    });
  });
}

async function uploadManyFilesThenPOST(files) {
  try {
    await Promise.all(files.map(fileURI => uploadFile('https://example.com/upload', fileURI)));
    const response = await fetch('https://example.com/confirmUploads', { method: 'POST' });
    if (!response.ok) throw new Error('Could not confirm uploads');
  } catch (error) {
    const response = await fetch('https://example.com/failedUploads', { method: 'POST' });
    if (!response.ok) throw new Error('Could not report failed uploads');
  }
  RNBackgroundUpload.canSuspendIfBackground();
}
```

The function `uploadManyFilesThenPOST` schedules all the file uploads at once. This is recommended because the OS can then make progress on all uploads even while your app sleeps. This may take some time as iOS decides to upload when it deems appropriate, e.g. when the device is charging and connected to WiFi. Inversely, when the device is low on battery or in energy saver mode, background uploads won't make progress.

When all uploads are finished, your app _may_ be resumed in the background to receive the events. You should call `canSuspendIfBackground` as soon as possible when you are done with other actions to conserve your app "background credit". If you don't call `canSuspendIfBackground`, the library will call it for your after ~45 seconds. This makes sure that your app won't be killed by the OS right away but pretty much consumes all your background credit.

Uploads tasks started when the app is in the background are [discretionary](https://developer.apple.com/documentation/foundation/nsurlsessionconfiguration/1411552-discretionary?language=objc); iOS will typically upload the files later when the device is charging. Upload tasks started in the foreground are not [discretionary](https://developer.apple.com/documentation/foundation/nsurlsessionconfiguration/1411552-discretionary?language=objc) and start right away. When your app is brought to the foreground, uploads that have been postponed by the OS will continue regardless of background credit.

If your app is dead when uploads complete (force-closed by the user via the app switcher or by the OS to reclaim memory), iOS will launch it in the background. The above example does not handle this case, i.e. there will be no `POST` to `https://example.com/confirmUploads`. To support this you should save the `uploadId`(s) to a file (e.g. via `AsyncStorage`), read it when your app starts, and add the 3 listeners back.


# iOS Background time request
Use this if background events are not enough and you need even more time.

```js
import RNBackgroundUpload from 'react-native-background-upload';

// Request background time. Do not call this on app suspend/resume since it might be already too late.
let taskId = await RNBackgroundUpload.beginBackgroundTask();

// Listen to background time is about to expire events. You can do some cleanup here. You will have about 3 to 4 seconds to run code
// before the app goes to sleep
let bgExpiredRelease = RNBackgroundUpload.addListener('bgExpired', null, (data) => {
  if(this.working && (!taskId || data.id == taskId)){
    // do some cleanup
  }
});

// run background code, do uploads, or even regular fetch requests

// tell the OS we are done. If you don't call this, your app will still be put to sleep internally
// to avoid it from being killed
if(taskId !== null){
  await RNBackgroundUpload.endBackgroundTask(taskId);
}
if(bgExpiredRelease){
  bgExpiredRelease.remove();
}

```

## Gratitude

Thanks to https://github.com/Vydia/react-native-background-upload to provide the main code and ideas.
