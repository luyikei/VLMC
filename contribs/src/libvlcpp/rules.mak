LIBVLCPP_VERSION := master
LIBVLCPP_URL := https://code.videolan.org/videolan/libvlcpp.git

PKGS += libvlcpp
ifeq ($(call need_pkg,"libvlcpp"),)
PKGS_FOUND += libvlcpp
endif

$(TARBALLS)/libvlcpp-$(LIBVLCPP_VERSION).tar.xz:
	$(call download_git,$(LIBVLCPP_URL),,$(LIBVLCPP_VERSION))

libvlcpp: libvlcpp-$(LIBVLCPP_VERSION).tar.xz
	$(UNPACK)
	$(MOVE)

.libvlcpp: libvlcpp
	$(RECONF)
	cd $< && $(HOSTVARS) ./configure $(HOSTCONF)
	cd $< && $(MAKE) install
	touch $@
	

