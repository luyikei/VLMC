
# CMake module to search for frei0r
# Author: Rohit Yadav <rohityadav89@gmail.com>
#
# If it's found it sets FREI0R_FOUND to TRUE
# and following variables are set:
#    FREI0R_INCLUDE_DIR

# Put here path to custom location
# example: /home/username/frei0r/include etc..
FIND_PATH( FREI0R_INCLUDE_DIR NAMES frei0r.h
  PATHS
    "$ENV{LIB_DIR}/include"
    "/usr/include"
    "/usr/include/frei0r"
    "/usr/local/include"
    "/usr/local/include/frei0r"
    # Mac OS
    "${CMAKE_CURRENT_SOURCE_DIR}/contribs/include"
    # MingW
    c:/msys/local/include
  )
FIND_PATH(FREI0R_INCLUDE_DIR PATHS "${CMAKE_INCLUDE_PATH}" NAMES frei0r.h)

# TODO: If required, add code to link to some library

IF (FREI0R_INCLUDE_DIR)
   SET(FREI0R_FOUND TRUE)
ENDIF (FREI0R_INCLUDE_DIR)

IF (FREI0R_FOUND)
   IF (NOT FREI0R_FIND_QUIETLY)
      MESSAGE(STATUS "Found frei0r include-dir path: ${FREI0R_INCLUDE_DIR}")
   ENDIF (NOT FREI0R_FIND_QUIETLY)
ELSE (FREI0R_FOUND)
   IF (FREI0R_FIND_REQUIRED)
      MESSAGE(FATAL_ERROR "Could not find frei0r")
   ENDIF (FREI0R_FIND_REQUIRED)
ENDIF (FREI0R_FOUND)

