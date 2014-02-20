list(APPEND VLMC_SRCS
	IBackend.h
	IRenderer.h
	ISource.h
		VLC/VLCBackend.cpp
		VLC/VLCRenderer.cpp
		VLC/VLCSource.cpp
			VLC/LibVLCpp/VLCInstance.cpp
			VLC/LibVLCpp/VLCMedia.cpp
			VLC/LibVLCpp/VLCMediaPlayer.cpp
			VLC/LibVLCpp/VLCpp.hpp
	)


