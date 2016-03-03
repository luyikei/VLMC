# Cross-compilation (from Unix)

## Get Contribs
 cd contribs
 sh contribs.sh

## Configure
mkdir win32 && cd win32
cmake -DCMAKE_TOOLCHAIN_FILE=../cmake/toolchain-win32.cmake ..

Use -D flag to set cmake flags such as:
cmake -DCMAKE_TOOLCHAIN_FILE=../cmake/toolchain-win32.cmake -DQT_MOC_EXECUTABLE=../contribs/tools/moc.exe ..

## Build
make

## Package It
make installer
