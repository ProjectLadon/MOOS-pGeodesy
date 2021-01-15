#--------------------------------------------------------
# The CMakeLists.txt for:                       pGeodesy
# Author(s):
#--------------------------------------------------------

SET(SRC
  Geodesy.cpp
  Geodesy_Info.cpp
  main.cpp
)

#=============================================================================
# FINDING MOOSGeodesy' HEADERS AND LIBRARIES...
#=============================================================================
find_package(MOOSGeodesy)
include_directories(${MOOSGeodesy_INCLUDE_DIRS})
link_directories(${MOOSGeodesy_LIBRARY_PATH})

message("+++++++++++++++++++++++++++++++++++++++++")
message("MOOSGeodesy_INCLUDE_DIRS:" ${MOOSGeodesy_INCLUDE_DIRS})
message("MOOSGeodesy_LIB_PATH:"     ${MOOSGeodesy_LIBRARY_PATH})
message("+++++++++++++++++++++++++++++++++++++++++")


INCLUDE_DIRECTORIES(${MOOS_INCLUDE_DIRS} ${MOOSGeodesy_INCLUDE_DIRS})

ADD_EXECUTABLE(pGeodesy ${SRC})

TARGET_LINK_LIBRARIES(pGeodesy
   ${MOOS_LIBRARIES}
   apputil
   mbutil
   m
   pthread
   proj
   MOOSGeodesy
)

find_program(CTAGS ctags)
if (CTAGS)
	FIND_FILE(MAKE_CTAGS make_ctags.sh ../..)
    ADD_CUSTOM_TARGET(geodesy_ctags ALL
        ${MAKE_CTAGS} .
        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
        DEPENDS pGeodesy
    )
endif(CTAGS)
