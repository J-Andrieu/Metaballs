# Once loaded this will define
#  MAGICK_FOUND        - system has GraphicsMagick
#  MAGICK_INCLUDE_DIR  - include directory for GraphicsMagick
#  MAGICK_LIBRARY_DIR  - library directory for GraphicsMagick
#  MAGICK_LIBRARIES    - libraries you need to link to
#
#  MAGICK++_FOUND        - system has GraphicsMagick
#  MAGICK++_INCLUDE_DIR  - include directory for GraphicsMagick
#  MAGICK++_LIBRARY_DIR  - library directory for GraphicsMagick
#  MAGICK++_LIBRARIES    - libraries you need to link to
#

SET(MAGICK_FOUND   0 )
SET(MAGICK++_FOUND 0 )

FIND_PATH( MAGICK_INCLUDE_DIR magick.h
  "$ENV{MAGICK_LOCATION}/magick"
  "$ENV{MAGICK_LOCATION}/include/GraphicsMagick"
  "$ENV{MAGICK_LOCATION}/include/magick"
  "$ENV{MAGICK_HOME}/include/magick"
  /usr/include/magick
  /usr/include/
  /usr/include/GraphicsMagick/
  /usr/include/GraphicsMagick/magick/
  /usr/local/include
  /usr/local/include/GraphicsMagick/magick
  /usr/local/include/GraphicsMagick/
  /opt/local/include/GraphicsMagick/magick
  /opt/local/include/GraphicsMagick
  )

FIND_PATH( MAGICK++_INCLUDE_DIR Magick++.h
  "$ENV{MAGICK++_LOCATION}/Magick++"
  "$ENV{MAGICK++_LOCATION}/include/"
  "$ENV{MAGICK_LOCATION}/Magick++"
  "$ENV{MAGICK_LOCATION}/include/Magick++"
  "$ENV{MAGICK_LOCATION}/include/GraphicsMagick"
  "$ENV{MAGICK_LOCATION}/include/"
  "$ENV{MAGICK_HOME}/include/"
  /usr/include/Magick++
  /usr/include/
  /usr/include/GraphicsMagick/
  /usr/local/include
  /usr/local/include/GraphicsMagick
  /opt/local/include/GraphicsMagick/Magick++
  /opt/local/include/GraphicsMagick
  )

FIND_LIBRARY( Magick GraphicsMagick
  PATHS 
  "$ENV{MAGICK_LOCATION}/magick/.libs"
  "$ENV{MAGICK_LOCATION}/lib"
  "$ENV{MAGICK_HOME}/lib"
  /usr/lib
  /usr/local/lib
  /opt/local/lib
  DOC   "GraphicsMagick magic library"
)


FIND_LIBRARY( Magick++ GraphicsMagick++
  PATHS 
  "$ENV{MAGICK++_LOCATION}/.libs"
  "$ENV{MAGICK_LOCATION}/.libs"
  "$ENV{MAGICK++_LOCATION}/lib"
  "$ENV{MAGICK_LOCATION}/lib"
  "$ENV{MAGICK_HOME}/lib"
  /opt/local/lib
  /usr/lib
  /usr/local/lib
   DOC   "GraphicsMagick Magick++ library"
)


SET(MAGICK_LIBRARIES ${Magick} )
SET(MAGICK++_LIBRARIES ${Magick++} )


IF (MAGICK_INCLUDE_DIR)
  IF(MAGICK_LIBRARIES)
    SET(MAGICK_FOUND 1)
    GET_FILENAME_COMPONENT(MAGICK_LIBRARY_DIR ${Magick}   PATH)
  ENDIF(MAGICK_LIBRARIES)
ENDIF(MAGICK_INCLUDE_DIR)

IF (MAGICK++_INCLUDE_DIR)
  IF(MAGICK++_LIBRARIES)
    SET(MAGICK++_FOUND 1)
    GET_FILENAME_COMPONENT(MAGICK++_LIBRARY_DIR ${Magick++} PATH)
  ENDIF(MAGICK++_LIBRARIES)
ENDIF(MAGICK++_INCLUDE_DIR)


IF(NOT MAGICK_FOUND)
  # make FIND_PACKAGE friendly
  IF(NOT Magick_FIND_QUIETLY)
    IF(Magick_FIND_REQUIRED)
      MESSAGE(FATAL_ERROR
              "GraphicsMagick required, please specify it's location with MAGICK_HOME, MAGICK_LOCATION or MAGICK++_LOCATION")
    ELSE(Magick_FIND_REQUIRED)
      MESSAGE(STATUS "GraphicsMagick was not found.")
    ENDIF(Magick_FIND_REQUIRED)
  ENDIF(NOT Magick_FIND_QUIETLY)
ENDIF(NOT MAGICK_FOUND)if(CMAKE_SIZEOF_VOID_P EQUAL 8)
	set(ASSIMP_ARCHITECTURE "64")
elseif(CMAKE_SIZEOF_VOID_P EQUAL 4)
	set(ASSIMP_ARCHITECTURE "32")
endif(CMAKE_SIZEOF_VOID_P EQUAL 8)
	
if(WIN32)
	set(ASSIMP_ROOT_DIR CACHE PATH "ASSIMP root directory")

	# Find path of each library
	find_path(ASSIMP_INCLUDE_DIR
		NAMES
			assimp/anim.h
		HINTS
			${ASSIMP_ROOT_DIR}/include
	)

	if(MSVC12)
		set(ASSIMP_MSVC_VERSION "vc120")
	elseif(MSVC14)	
		set(ASSIMP_MSVC_VERSION "vc140")
	endif(MSVC12)
	
	if(MSVC12 OR MSVC14)
	
		find_path(ASSIMP_LIBRARY_DIR
			NAMES
				assimp-${ASSIMP_MSVC_VERSION}-mt.lib
			HINTS
				${ASSIMP_ROOT_DIR}/lib${ASSIMP_ARCHITECTURE}
		)
		
		find_library(ASSIMP_LIBRARY_RELEASE				assimp-${ASSIMP_MSVC_VERSION}-mt.lib 			PATHS ${ASSIMP_LIBRARY_DIR})
		find_library(ASSIMP_LIBRARY_DEBUG				assimp-${ASSIMP_MSVC_VERSION}-mtd.lib			PATHS ${ASSIMP_LIBRARY_DIR})
		
		set(ASSIMP_LIBRARY 
			optimized 	${ASSIMP_LIBRARY_RELEASE}
			debug		${ASSIMP_LIBRARY_DEBUG}
		)
		
		set(ASSIMP_LIBRARIES "ASSIMP_LIBRARY_RELEASE" "ASSIMP_LIBRARY_DEBUG")
	
		FUNCTION(ASSIMP_COPY_BINARIES TargetDirectory)
			ADD_CUSTOM_TARGET(AssimpCopyBinaries
				COMMAND ${CMAKE_COMMAND} -E copy ${ASSIMP_ROOT_DIR}/bin${ASSIMP_ARCHITECTURE}/assimp-${ASSIMP_MSVC_VERSION}-mtd.dll 	${TargetDirectory}/Debug/assimp-${ASSIMP_MSVC_VERSION}-mtd.dll
				COMMAND ${CMAKE_COMMAND} -E copy ${ASSIMP_ROOT_DIR}/bin${ASSIMP_ARCHITECTURE}/assimp-${ASSIMP_MSVC_VERSION}-mt.dll 		${TargetDirectory}/Release/assimp-${ASSIMP_MSVC_VERSION}-mt.dll
			COMMENT "Copying Assimp binaries to '${TargetDirectory}'"
			VERBATIM)
		ENDFUNCTION(ASSIMP_COPY_BINARIES)
	
	endif()
	
else(WIN32)

	find_path(
	  assimp_INCLUDE_DIRS
	  NAMES postprocess.h scene.h version.h config.h cimport.h
	  PATHS 
	# S E A R C H   E V E R Y T H I N G
	/usr/local/include/
	/usr/local/include/assimp
	/usr/local/include
	/usr/include/assimp
	/usr/include
	)

	find_library(
	  assimp_LIBRARIES
	  NAMES assimp
	  PATHS 
	# S E A R C H   E V E R Y T H I N G	
	/usr/local/lib/
	/usr/local/lib
	/usr/lib
    	/usr/lib64
	/sw/lib
	/opt/local/lib
	/opt/csw/lib
	/opt/lib
	)

	if (assimp_INCLUDE_DIRS AND assimp_LIBRARIES)
	  SET(assimp_FOUND TRUE)
	ENDIF (assimp_INCLUDE_DIRS AND assimp_LIBRARIES)

	if (assimp_FOUND)
	  if (NOT assimp_FIND_QUIETLY)
		message(STATUS "Found asset importer library: ${assimp_LIBRARIES}")
	  endif (NOT assimp_FIND_QUIETLY)
	else (assimp_FOUND)
	  if (assimp_FIND_REQUIRED)
		message(FATAL_ERROR "Could not find asset importer library")
	  endif (assimp_FIND_REQUIRED)
	endif (assimp_FOUND)
	
endif(WIN32)
