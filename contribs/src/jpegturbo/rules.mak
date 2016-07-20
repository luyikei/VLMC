# jpeg

JPEGTURBO_VERSION := 1.5.0
JPEGTURBO_URL := http://downloads.sourceforge.net/project/libjpeg-turbo/$(JPEGTURBO_VERSION)/libjpeg-turbo-$(JPEGTURBO_VERSION).tar.gz

$(TARBALLS)/libjpeg-turbo-$(JPEGTURBO_VERSION).tar.gz:
	$(call download_pkg,$(JPEGTURBO_URL),jpegturbo)

.sum-jpegturbo: libjpeg-turbo-$(JPEGTURBO_VERSION).tar.gz

jpegturbo: libjpeg-turbo-$(JPEGTURBO_VERSION).tar.gz .sum-jpegturbo
	$(UNPACK)
	$(UPDATE_AUTOCONFIG)
	$(MOVE)

.jpegturbo: jpegturbo
	cd $< && $(HOSTVARS) ./configure $(HOSTCONF)
	cd $< && $(MAKE) install
	touch $@
