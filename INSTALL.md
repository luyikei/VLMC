# Building VLMC

## Dependencies
* Latest version of VLC installed (from the git repository)
* Qt framework >= 5.6.0
* MLT >= 6.4.0
* medialibrary
* libvlcpp


## Install VLC

	See https://wiki.videolan.org/Category:Building/

## Install libvlcpp

	git clone https://code.videolan.org/videolan/libvlcpp.git
	cd libvlcpp
	./bootstrap
	./configure
	sudo make install


## Install medialibrary

	git clone https://code.videolan.org/videolan/medialibrary.git
	cd medialibrary
	./bootstrap
	./configure
	make
	sudo make install

## Install MLT

	git clone https://github.com/mltframework/mlt.git
	cd mlt
	./configure
	make
	sudo make install

## Build VLMC

	git clone https://code.videolan.org/videolan/vlmc.git
	cd vlmc
	./bootstrap
	./configure
	make

## Running VLMC

	./vlmc

## Issues

If you run into problems, you can ask for help on <vlmc-devel@videolan.org> and
IRC channel #vlmc on Freenode.
