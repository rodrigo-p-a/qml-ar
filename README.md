## AR module for QML
This module allows to add an AR component to your QML application simply by importing our library. The framework uses <a href="http://limu.ait.kyushu-u.ac.jp/~uchiyama/me/code/UCHIYAMARKERS/index.html">Random dot markers</a> by Hideaki Uchiyama, which are chosen over other possibilities due to their small negative visual impact on the scene they are put on.

## Gallery:
Android:
<p>
<img src="/example/screenshots/android1.jpg?raw=true" height="200" alt="Sample application on Android" />
<img src="/example/screenshots/android2.jpg?raw=true" height="200" alt="Sample application on Android" />
<img src="/example/screenshots/android3.jpg?raw=true" height="200" alt="Sample application on Android" />
<img src="/example/screenshots/android4.jpg?raw=true" height="200" alt="Sample application on Android" />
</p>

Linux:
<p>
<img src="/example/screenshots/linux1.jpg?raw=true" height="200" alt="Sample application on Linux" />
<img src="/example/screenshots/linux2.jpg?raw=true" height="200" alt="Sample application on Linux" />
<img src="/example/screenshots/linux3.jpg?raw=true" height="200" alt="Sample application on Linux" />
</p>

## Prerequisites (Common)
1. <a href="https://www.qt.io/download">Qt 5</a> (tested with 5.10.0) and Qt Creator (tested with 4.5.0), tested on Desktop and Android kits
2. git

## Prerequisites (Linux)
1. <a href="https://github.com/opencv/opencv">OpenCV for Desktop</a> (tested with bf418ba34)
2. Tested on 4xAMD A8-7410 APU (GPU not used)

## Prerequisites (Android)
1. <a href="https://sourceforge.net/projects/opencvlibrary/files/opencv-android/">OpenCV for Android</a> (tested with 3.3.1)
2. Android SDK (tested with r22.3) and NDK (tested with r15c), need to be set up with Qt
3. Android device (tested with Samsung Galaxy SM-T810 running on cyanogenmod) *Up to 5 cores required, GPU is used for image rendering*

## Installation
1. `git submodule init && git submodule update`
2. Create a symlink called `opencv-android` pointing to the OpenCV for Android
3. `mkdir build_linux; cd build_linux; qmake ..; make -j10 install`
4. Same for Android, also `export ANDROID_NDK_ROOT=/path/to/ndk` required
5. Open Qt Creator, open `example/ar-chest.pro`, compile and run on Desktop/Android

## Running
1. Download pdf and print the example file with markers:<br /><a href="/example/demo_sheet/ar_demo_sheet.pdf"><img src="/example/demo_sheet/ar_demo_sheet.png" width="200" /></a>
2. Attach camera to your PC (device 1 is used by default). Point camera towards the printed sheet
3. A chest from Qt examples and a cuboid should appear

## Application Structure
<img src="/doc/components.png" />

## Adding markers to your scene
You can add random dot markers to your `svg` file by using our tool: <a href="https://github.com/chili-epfl/qml-ar-inkscape">qml-ar-inkscape</a>

## Contact
<a href="mailto:sergei.volodin@epfl.ch">Sergei Volodin</a>, <a href="mailto:wafa.johal@gmail.com">Wafa Johal</a>, EPFL, <a href="http://chili.epfl.ch">CHILI lab</a>
