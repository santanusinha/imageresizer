cmake_minimum_required(VERSION 3.1.0 FATAL_ERROR)

project(imageresizer CXX)

find_package(PkgConfig)
find_package(ImageMagick)
find_package(ImageMagick COMPONENTS Magick++ REQUIRED)

pkg_check_modules(YAML QUIET yaml-cpp)

set( CMAKE_CXX_FLAGS   "${CMAKE_CXX_FLAGS} -fstack-protector -fstack-protector-all -Wall -Werror -DMAGICKCORE_HDRI_ENABLE=0 -DMAGICKCORE_QUANTUM_DEPTH=16" )
set( CMAKE_C_FLAGS_DEBUG   "-O0 -ggdb -DDEBUG" )
set( CMAKE_C_FLAGS_RELEASE "-O4" )

set(PISTASCHE_HOME "/usr/local/")
set(YAML_HOME "/usr/local/")

include_directories(${PISTASCHE_HOME}/include ${ImageMagick_INCLUDE_DIRS} ${YAML_HOME}/include )

add_executable(imageresizer threadcounter.cpp datamanager.cpp resizer.cpp main.cpp)

target_link_libraries (imageresizer ${PISTASCHE_HOME}/lib/libnet_static.a ${ImageMagick_LIBRARIES} ${YAML_HOME}/lib/libyaml-cpp.a pthread)

target_compile_features(imageresizer PRIVATE cxx_range_for)
