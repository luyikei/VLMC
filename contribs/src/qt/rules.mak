# Qt

QT_VERSION = 5.6.0
QT_URL := https://download.qt.io/official_releases/qt/5.6/$(QT_VERSION)/single/qt-everywhere-opensource-src-$(QT_VERSION).tar.xz

ifdef HAVE_MACOSX
#PKGS += qt
endif
ifdef HAVE_WIN32
PKGS += qt
endif

ifeq ($(call need_pkg,"Qt5Core Qt5Gui Qt5Widgets Qt5Network Qt5Qml"),)
PKGS_FOUND += qt
endif

$(TARBALLS)/qt-$(QT_VERSION).tar.xz:
	$(call download,$(QT_URL))

.sum-qt: qt-$(QT_VERSION).tar.xz

qt: qt-$(QT_VERSION).tar.xz .sum-qt
	$(UNPACK)
	mv qt-everywhere-opensource-src-$(QT_VERSION) qt-$(QT_VERSION)
	$(MOVE)

ifdef HAVE_MACOSX
QT_PLATFORM := -platform darwin-g++
endif
ifdef HAVE_WIN32
QT_SPEC := win32-g++
QT_PLATFORM := -xplatform win32-g++ -device-option CROSS_COMPILE=$(HOST)-
endif

QT_CONFIG := -static -release -opensource -confirm-license -no-pkg-config \
	-no-sql-sqlite -no-gif -qt-libjpeg -no-openssl -no-opengl -no-dbus \
	-no-qml-debug -no-audio-backend -no-sql-odbc \
	-no-compile-examples -nomake examples -skip serialbus -skip activeqt \
	-skip multimedia -skip wayland -skip serialport -skip qtwinextras \
	-skip qtsensors

.qt: qt
	cd $< && ./configure $(QT_PLATFORM) $(QT_CONFIG) -prefix $(PREFIX)
	# Make && Install libraries
	cd $< && $(MAKE)
	cd $</qtbase/src && $(MAKE) sub-corelib-install_subtargets sub-gui-install_subtargets sub-widgets-install_subtargets sub-platformsupport-install_subtargets sub-network-install_subtargets
	# Install tools
	cd $</qtbase/src && $(MAKE) sub-moc-install_subtargets sub-rcc-install_subtargets sub-uic-install_subtargets
	# Install plugins
	cd $</qtbase/src/plugins && $(MAKE) sub-platforms-install_subtargets
	mv $(PREFIX)/plugins/platforms/libqwindows.a $(PREFIX)/lib/ && rm -rf $(PREFIX)/plugins
	cd $</qtdeclarative && $(MAKE) sub-src-install_subtargets
	# Move includes to match what VLC expects
	mkdir -p $(PREFIX)/include/QtGui/qpa
	cp $(PREFIX)/include/QtGui/$(QT_VERSION)/QtGui/qpa/qplatformnativeinterface.h $(PREFIX)/include/QtGui/qpa
	# Clean Qt mess
	rm -rf $(PREFIX)/lib/libQt5Bootstrap* $(PREFIX)/lib/*.prl $(PREFIX)/mkspecs
	# Fix .pc files to remove debug version (d)
	cd $(PREFIX)/lib/pkgconfig; for i in Qt5Core.pc Qt5Gui.pc Qt5Widgets.pc Qt5Qml.pc; do sed -i -e 's/d\.a/.a/g' -e 's/d $$/ /' $$i; done
	# Fix Qt5Gui.pc file to include qwindows (QWindowsIntegrationPlugin) and Qt5Platform Support
	cd $(PREFIX)/lib/pkgconfig; sed -i -e 's/ -lQt5Gui/ -lqwindows -lQt5PlatformSupport -lQt5Gui/g' Qt5Gui.pc
ifdef HAVE_CROSS_COMPILE
	# Building Qt build tools for Xcompilation
	cd $</qtbase/include/QtCore; ln -sf $(QT_VERSION)/QtCore/private
	cd $</qtbase/qmake; $(MAKE)
	cd $</qtbase/src/tools; \
	for i in bootstrap uic rcc moc; \
		do (cd $$i; echo $$i && ../../../bin/qmake -spec $(QT_SPEC) && $(MAKE) clean && $(MAKE) CC=$(HOST)-gcc CXX=$(HOST)-g++ LINKER=$(HOST)-g++ && $(MAKE) install); \
	done
endif
	touch $@
