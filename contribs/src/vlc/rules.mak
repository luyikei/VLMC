ifdef HAVE_WIN32
FLAVOR:=win32
else
ifdef HAVE_WIN64
FLAVOR:=win64
endif
endif

VLC_VERSION := 3.0.0-20160720-0235
VLC_ARCHIVE_NAME := vlc-$(VLC_VERSION)-git-$(FLAVOR).7z
VLC_URL := http://nightlies.videolan.org/build/$(FLAVOR)/last/$(VLC_ARCHIVE_NAME)

PKGS += vlc
ifeq ($(call need_pkg,"libvlc"),)
PKGS_FOUND += vlc
endif

$(TARBALLS)/$(VLC_ARCHIVE_NAME):
	$(call download_pkg,$(VLC_URL),vlc)

vlc: $(VLC_ARCHIVE_NAME)
	$(UNPACK)
	mv vlc-3.0.0-git vlc-$(VLC_VERSION)-git-$(FLAVOR)
	$(MOVE)

.vlc: vlc
	cd $< && cp -R sdk/include $(PREFIX)/
	cd $< && cp -R sdk/lib $(PREFIX)/
	touch $@
