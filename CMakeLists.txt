cmake_minimum_required(VERSION 2.8 FATAL_ERROR)
# cmake_minimum_required(VERSION 3.8 FATAL_ERROR)


set (PROJECT histreamer) 
project (${PROJECT})
# project(histreamer)

# find_package(pjsip-hisi 2.9 REQUIRED)
# -DPJ_AUTOCONF=1 -DPJ_IS_LITTLE_ENDIAN=1 -DPJ_IS_BIG_ENDIAN=0 
# add_compile_definitions(PJ_AUTOCONF=1)
# add_compile_definitions(PJ_IS_LITTLE_ENDIAN=1)
# add_compile_definitions(PJ_IS_BIG_ENDIAN=0)

MESSAGE(STATUS "Build type: " ${CMAKE_BUILD_TYPE})
MESSAGE("CMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE}")
MESSAGE("CMAKE_CONFIGURATION_TYPES=${CMAKE_CONFIGURATION_TYPES}")

MESSAGE("CMAKE_BUILD_TYPE = ${CMAKE_BUILD_TYPE}")

IF(CMAKE_CONFIGURATION_TYPES MATCHES audio)
	MESSAGE("Audio streamer build.")
	set(SOURCE_LIB audio_stream_util.c)
	add_executable(${PROJECT}-audio ${PROJECT}-audio.c)
	add_library(audio_stream_util STATIC ${SOURCE_LIB})
	target_compile_definitions(${PROJECT}-audio PRIVATE PJ_AUTOCONF=1 PJ_IS_LITTLE_ENDIAN=1 PJ_IS_BIG_ENDIAN=0)
	target_compile_definitions(audio_stream_util PRIVATE PJ_AUTOCONF=1 PJ_IS_LITTLE_ENDIAN=1 PJ_IS_BIG_ENDIAN=0)
	target_link_libraries(${PROJECT}-audio pj pjlib-util pjmedia-audiodev pjmedia-codec pjmedia audio_stream_util)
	install(TARGETS ${PROJECT}-audio DESTINATION bin)
ELSEIF(CMAKE_CONFIGURATION_TYPES MATCHES video)
	MESSAGE("Video streamer build.")
	add_executable(${PROJECT}-video ${PROJECT}-video.c)
	target_compile_definitions(${PROJECT}-video PRIVATE PJ_AUTOCONF=1 PJ_IS_LITTLE_ENDIAN=1 PJ_IS_BIG_ENDIAN=0)
	target_link_libraries(${PROJECT}-video pj pjlib-util)
	install(TARGETS ${PROJECT}-video DESTINATION bin)
ELSE()
	MESSAGE(FATAL_ERROR "build type not selected")
ENDIF()