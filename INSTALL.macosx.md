# Building VLMC on Mac OSX

### Dependencies:
- Build tools: Xcode command-line tools
- homebrew

### Get the sources:
  git clone https://code.videolan.org/videolan/vlmc.git

## Building and Packaging

### Get the actual dependencies:
* brew install cmake
* brew install Qt5
* brew install frei0r
* ./contribs/build-libvlc-for-mac.sh
* git submodule init
* git submodule update

### Compile vlmc:
Now cd to root source directory and build:
  mkdir build && cd build
  cmake ..
  make

This will by default create a Mac Bundle, vlmc.app in /build/bin

### To create a dmg image:
 Uncomment #dmg in /src/CMakeLists.txt, at the end of the file.
 and follow the build process, the dmg will be created in /build/bin
