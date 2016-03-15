#
# VLMC build system
# Authors: Ludovic Fauvet <etix@vlmc.org>
#          Rohit Yadav <rohityadav89@gmail.com>
#

cmake_minimum_required(VERSION 3.1)
## RCC file
SET(TS_QRC ${CMAKE_SOURCE_DIR}/ts/resources-ts.qrc)

if(Qt5LinguistTools_FOUND)

# Don't remove TS files on make clean.
set_directory_properties(PROPERTIES CLEAN_NO_CUSTOM 1)

## Set availbale translations file ids here
OPTION(LANGUAGES "Using translations")
SET(ALL_LANGUAGES
        ca
        cs
        de
        es
        eu
        fr
        gl
        el
        hu
        it
        ja
        nl
        pl
        pt
        ro
        ru
        sl
        sk
        sv
        ta
        te
        tr
        uk
        zh
   )

## Check if user has provided specific LANGS option
IF(NOT DEFINED LANGS)
    SET(LANGUAGES ${ALL_LANGUAGES} CACHE STRING "Using translations" FORCE)
ELSE(NOT DEFINED LANGS)
    IF(NOT LANGS)
        SET(LANGUAGES "" CACHE STRING "Using translations" FORCE)
    ELSEIF(LANGS STREQUAL *)
        SET(LANGUAGES ${ALL_LANGUAGES} CACHE STRING "Using translations" FORCE)
    ELSE(NOT LANGS)
        STRING(REGEX MATCHALL [a-zA-Z_]+
               langs1 ${LANGS})
        SET(LANGUAGES ${langs1} CACHE STRING "Using translations" FORCE)
    ENDIF(NOT LANGS)
ENDIF(NOT DEFINED LANGS)

## Display what translations files will be processed
MESSAGE(STATUS "Translations: ${LANGUAGES}")

## Collect all translations files that are to be processed
FOREACH(LANGUAGE ${LANGUAGES})
    FILE(GLOB temp_TS ${CMAKE_SOURCE_DIR}/ts/vlmc*${LANGUAGE}*.ts)
    LIST(APPEND vlmc_TS ${temp_TS})
ENDFOREACH()

## Wraps the collected translations
qt5_add_translation(vlmc_QMS ${vlmc_TS})

## Create a target that runs lrelease for all the .ts files
## "ALL" means that it will be run by default.
ADD_CUSTOM_TARGET(translations ALL DEPENDS ${vlmc_QMS})

## Search for all translatable strings in the sources directory
qt5_create_translation(vlmc_TS_UPDATE ${CMAKE_SOURCE_DIR}/src ${vlmc_TS})

## Create a target that runs lupdate for all the sources and UI files
message(STATUS "ts: ${vlmc_TS_UPDATE}")
ADD_CUSTOM_TARGET(lupdate DEPENDS ${vlmc_TS_UPDATE})

else()
    MESSAGE(WARNING "VLMC could not find Qt5 linguist tools. i18n support will be disabled.")
    ## Create an empty RCC file
    FILE(WRITE ${TS_QRC} "<RCC></RCC>")
endif()
