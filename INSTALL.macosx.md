# Building VLMC on Mac OSX

### Dependencies:
- Build tools: Xcode command-line tools
- homebrew

### Get the sources:
  git clone https://code.videolan.org/videolan/vlmc.git

## Building and Packaging

### Get the actual dependencies:
* brew tap tomahawk-player/tomahawkqt5
* brew install vlc
* brew install qt4
* brew install frei0r


### Compile vlmc:
Now cd to root source directory and build:
  mkdir build && cd build
  cmake ..
  make

This will by default create a Mac Bundle, vlmc.app in /build/bin

### To create a dmg image:
 Uncomment #dmg in /src/CMakeLists.txt, at the end of the file.
 and follow the build process, the dmg will be created in /build/bin
