MEDIALIBRARY_VERSION := master
MEDIALIBRARY_URL := https://code.videolan.org/videolan/medialibrary.git

#PKGS += medialibrary
ifeq ($(call need_pkg,"medialibrary"),)
PKGS_FOUND += medialibrary
endif

DEPS_medialibrary += vlc jpegturbo sqlite

$(TARBALLS)/medialibrary-$(MEDIALIBRARY_VERSION).tar.xz:
	$(call download_git,$(MEDIALIBRARY_URL),,$(MEDIALIBRARY_VERSION))

medialibrary: medialibrary-$(MEDIALIBRARY_VERSION).tar.xz
	$(UNPACK)
	$(MOVE)

.medialibrary: medialibrary
	cd $< && ./bootstrap
	cd $< && $(HOSTVARS) ./configure $(HOSTCONF)
	cd $< && $(MAKE) install
	touch $@
