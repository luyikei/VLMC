project(vlmc)

include(ExternalProject)

set( PREFIX ${PROJECT_BINARY_DIR}/contribs )
set( CONTRIBS_DIR ${PREFIX}/install PARENT_SCOPE )

# Find libvlcpp
ExternalProject_Add(
    libvlcpp
    PREFIX ${PREFIX}
    UPDATE_COMMAND ""
    GIT_REPOSITORY https://code.videolan.org/videolan/libvlcpp.git
    CONFIGURE_COMMAND <SOURCE_DIR>/bootstrap && <SOURCE_DIR>/configure --prefix=${PREFIX}/install --disable-examples
    BUILD_COMMAND make
    INSTALL_COMMAND make install
)

# Now find medialibrary
ExternalProject_Add(
    medialibrary
    PREFIX ${PREFIX}
    UPDATE_COMMAND ""
    GIT_REPOSITORY https://code.videolan.org/videolan/medialibrary.git
    CONFIGURE_COMMAND <SOURCE_DIR>/bootstrap && <SOURCE_DIR>/configure --prefix=${PREFIX}/install --disable-tests
    BUILD_COMMAND make
    INSTALL_COMMAND make install
)
