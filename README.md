# Passes
Access .pkpass passes on Ubuntu Touch

[![OpenStore](https://open-store.io/badges/en_US.png)](https://open-store.io/app/passes.s710)

## Features

This app allows to import passes in .pkpass format and display them in a list, similar to iOS' wallet. Each pass will be shown with it's entire information, including the respective code which can then be scanned.

- Import passes (.pkass files) into the app
- Share passes from within the app
- Fetch updates for a pass (in case it provides a webservice for updates)

To display a single pass, tap its card. The `i` icon on the bottom right allows to show the back of the card, which might provide additional information.

Expired passes are automatically hidden, but can be displayed at any time, if desired.

The app is locked in portrait mode (on purpose), please don't ask to support landscape mode.
The app has been tested with various passes, however it might still have issues with formats I have not yet encountered. Likewise, I have only tested it on a single device. While the app should resize to all device dimensions correctly, other devices might still have displaying issues.

If you encounter any issue with the app, **please report on github**. Do not report errors via openstore reviews, as I cannot respond to those.

<p float="left">
<img title="Screenshot" alt="Screenshot" width="32%" src="screenshots/screenshot1.png">
<img title="Screenshot" alt="Screenshot" width="32%" src="screenshots/screenshot2.png">
<img title="Screenshot" alt="Screenshot" width="32%" src="screenshots/screenshot3.png">

</p>

## Reference

Implementation based on the specification described here:
- https://developer.apple.com/library/archive/documentation/UserExperience/Reference/PassKit_Bundle/Chapters/TopLevel.html#//apple_ref/doc/uid/TP40012026-CH2-SW2
- https://developer.apple.com/library/archive/documentation/UserExperience/Reference/PassKit_Bundle/Chapters/PackageStructure.html#//apple_ref/doc/uid/TP40012026-CH1-SW1
- https://developer.apple.com/library/archive/documentation/UserExperience/Reference/PassKit_Bundle/Chapters/LowerLevel.html#//apple_ref/doc/uid/TP40012026-CH3-SW3
- https://developer.apple.com/library/archive/documentation/UserExperience/Reference/PassKit_Bundle/Chapters/FieldDictionary.html
- https://developer.apple.com/library/archive/documentation/PassKit/Reference/PassKit_WebService/WebService.html#//apple_ref/doc/uid/TP40011988


# License

Copyright (C) 2021-2022 Patrick Fial

Licensed under the MIT license

# Copyright notice
- Pass icon in app icon made by <a href="https://www.freepik.com" title="Freepik">Freepik</a> from <a href="https://www.flaticon.com/" title="Flaticon">www.flaticon.com</a>

- This app uses the [zxing](https://github.com/nu-book/zxing-cpp.git) libraries for generating barcodes.

- This app uses the [Quazip](https://github.com/stachenov/quazip.git) library for opening compressed archives.

# Building this app

Create a fork of this repository, and clone locally with `git clone LINK`.
Then get the submodules with `git submodule update --init`.
Next you need to build the submodules with `clickable build --libs quazip --arch arm64` and `clickable build --libs zxing-cpp --arch arm64`. Or simply `clickable build --libs --arch arm64` for both. Use the target architecture you are building for.
Now you are ready to build, install and start the app with `clickable`.