cmake_minimum_required(VERSION 3.1.0 FATAL_ERROR)

project(imageresizer CXX)

find_package(PkgConfig)
find_package(ImageMagick)
find_package(ImageMagick COMPONENTS Magick++ REQUIRED)
find_package(Boost 1.58 COMPONENTS program_options REQUIRED)

pkg_check_modules(YAML REQUIRED yaml-cpp)

set( CMAKE_CXX_FLAGS   "${CMAKE_CXX_FLAGS} -fstack-protector -fstack-protector-all -Wall -Werror -DMAGICKCORE_HDRI_ENABLE=0 -DMAGICKCORE_QUANTUM_DEPTH=16" )
set( CMAKE_C_FLAGS_DEBUG   "-O0 -ggdb -DDEBUG" )
set( CMAKE_C_FLAGS_RELEASE "-O4 -Wno-error" )

add_subdirectory(external/googletest)
#add_subdirectory(external/yaml-cpp)
add_subdirectory(external/pistache)

include_directories(external/pistache/include ${pistache_INCLUDE_DIR} ${YAML_INCLUDE_DIRS} ${ImageMagick_INCLUDE_DIRS} ${Boost_INCLUDE_DIR}  external/spdlog/include)

add_executable(imageresizer accesslog.cpp util.cpp config.cpp threadcounter.cpp datamanager.cpp resizer.cpp main.cpp)

target_link_libraries (imageresizer net_static ${YAML_STATIC_LIBRARIES} ${ImageMagick_LIBRARIES} ${Boost_LIBRARIES} pthread)

target_compile_features(imageresizer PRIVATE cxx_range_for)
