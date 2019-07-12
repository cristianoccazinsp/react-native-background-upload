# Forked repo from https://github.com/Vydia/react-native-background-upload used to remove android support (which is outdated and can be achieved with just fetch and a foreground service). Also updated to allow non file uploads with multipart

# react-native-background-upload [![npm version](https://badge.fury.io/js/react-native-background-upload.svg)](https://badge.fury.io/js/react-native-background-upload)
The only React Native http post file uploader with iOS background support.  If you are uploading large files like videos, use this so your users can background your app during a long upload.

NOTE: Use major version 4 with RN 47.0 and greater.  If you have RN less than 47, use 3.0.  To view all available versions:
`npm show react-native-background-upload versions`


# Installation

## 1. Install package

`npm install --save react-native-background-upload`

or

`yarn add react-native-background-upload`

Note: To install from this repo, set on package.json:
  "react-native-background-upload": "github:cristianoccazinsp/react-native-background-upload"

## 2. Link Native Code

### Automatic Native Library Linking

`react-native link react-native-background-upload`

### Or, Manually Link It

#### iOS

1. In the XCode's "Project navigator", right click on your project's Libraries folder ➜ `Add Files to <...>`
2. Go to `node_modules` ➜ `react-native-background-upload` ➜ `ios` ➜ select `VydiaRNFileUploader.xcodeproj`
3. Add `VydiaRNFileUploader.a` to `Build Phases -> Link Binary With Libraries`

## 3. Expo

To use this library with [Expo](https://expo.io) one must first detach (eject) the project and follow [step 2](#2-link-native-code) instructions. Additionally on iOS there is a must to add a Header Search Path to other dependencies which are managed using Pods. To do so one has to add `$(SRCROOT)/../../../ios/Pods/Headers/Public` to Header Search Path in `VydiaRNFileUploader` module using XCode.

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
    // data includes responseCode: number and responseBody: Object
    console.log('Completed!')
  })
}).catch((err) => {
  console.log('Upload error!', err)
})
```

## Multipart Uploads

Just set the `type` option to `multipart` and set the `field` option.  Example:

```
const options = {
  url: 'https://myservice.com/path/to/post',
  path: 'file://path/to/file%20on%20device.png',
  method: 'POST',
  field: 'uploaded_media',
  type: 'multipart'
}
```

Note the `field` property is required for multipart uploads.

# API

## Top Level Functions

All top-level methods are available as named exports or methods on the default export.

### startUpload(options)

The primary method you will use, this starts the upload process.

Returns a promise with the string ID of the upload.  Will reject if there is a connection problem, the file doesn't exist, or there is some other problem.

`options` is an object with following values:

*Note: You must provide valid URIs. react-native-background-upload does not escape the values you provide.*

|Name|Type|Required|Default|Description|Example|
|---|---|---|---|---|---|
|`url`|string|Required||URL to upload to|`https://myservice.com/path/to/post`|
|`path`|string|Required||File path on device|`file://something/coming/from%20the%20device.png`|
|`type`|'raw' or 'multipart'|Optional|`raw`|Primary upload type.||
|`method`|string|Optional|`POST`|HTTP method||
|`customUploadId`|string|Optional||`startUpload` returns a Promise that includes the upload ID, which can be used for future status checks.  By default, the upload ID is automatically generated.  This parameter allows a custom ID to use instead of the default.||
|`headers`|object|Optional||HTTP headers|`{ 'Accept': 'application/json' }`|
|`field`|string|Required if `type: 'multipart'`||The form field name for the file.  Only used when `type: 'multipart`|`uploaded-file`|
|`parameters`|object|Optional||Additional form fields to include in the HTTP request. Only used when `type: 'multipart`||



### getFileInfo(path)

Returns some useful information about the file in question.  Useful if you want to set a MIME type header.

`path` is a string, such as `file://path.to.the.file.png`

Returns a Promise that resolves to an object containing:

|Name|Type|Required|Description|Example|
|---|---|---|---|---|
|`name`|string|Required|The file name within its directory.|`image2.png`|
|`exists`|boolean|Required|Is there a file matching this path?||
|`size`|number|If `exists`|File size, in bytes||
|`extension`|string|If `exists`|File extension|`mov`|
|`mimeType`|string|If `exists`|The MIME type for the file.|`video/mp4`|

### cancelUpload(uploadId)

Cancels an upload.

`uploadId` is the result of the Promise returned from `startUpload`

Returns a Promise that resolves to an boolean indicating whether the upload was cancelled.

### addListener(eventType, uploadId, listener)

Adds an event listener, possibly confined to a single upload.

`eventType` Event to listen for. Values: 'progress' | 'error' | 'completed' | 'cancelled'

`uploadId` The upload ID from `startUpload` to filter events for.  If null, this will include all uploads.

`listener` Function to call when the event occurs.

Returns an [EventSubscription](https://github.com/facebook/react-native/blob/master/Libraries/vendor/emitter/EmitterSubscription.js). To remove the listener, call `remove()` on the `EventSubscription`.

## Events

### progress

Event Data

|Name|Type|Required|Description|
|---|---|---|---|
|`id`|string|Required|The ID of the upload.|
|`progress`|0-100|Required|Percentage completed.|

### error

Event Data

|Name|Type|Required|Description|
|---|---|---|---|
|`id`|string|Required|The ID of the upload.|
|`error`|string|Required|Error message.|

### completed

Event Data

|Name|Type|Required|Description|
|---|---|---|---|
|`id`|string|Required|The ID of the upload.|
|`responseCode`|string|Required|HTTP status code received|
|`responseBody`|string|Required|HTTP response body|

### cancelled

Event Data

|Name|Type|Required|Description|
|---|---|---|---|
|`id`|string|Required|The ID of the upload.|
