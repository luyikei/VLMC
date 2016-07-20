MLT_VERSION := master
MLT_URL := https://github.com/mltframework/mlt.git

PKGS += mlt
ifeq ($(call need_pkg,"mltframework"),)
PKGS_FOUND += mlt
endif

DEPS_mlt += ffmpeg iconv sdl

ifdef HAVE_WIN32
MLT_CONFIG=--target-os=mingw
endif

$(TARBALLS)/mlt-$(MLT_VERSION).tar.xz:
	$(call download_git,$(MLT_URL),,$(MLT_VERSION))

mlt: mlt-$(MLT_VERSION).tar.xz
	$(UNPACK)
	$(MOVE)

.mlt: mlt
	cd $< && $(HOSTVARS) ./configure $(HOSTCONF) $(MLT_CONFIG)
	cd $< && $(MAKE) && $(MAKE) install
	touch $@
