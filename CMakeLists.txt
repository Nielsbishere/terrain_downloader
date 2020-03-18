cmake_minimum_required(VERSION 3.13.0)
project(heightmap_extractor)

set_property(GLOBAL PROPERTY USE_FOLDERS ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

file(GLOB_RECURSE hpp "include/*.hpp")
file(GLOB_RECURSE cpp "src/*.cpp")

add_subdirectory(curl)
add_subdirectory(libzip)

add_executable(
	heightmap_extractor
	${hpp}
	${cpp}
	CMakeLists.txt
)

set_property(TARGET heightmap_extractor PROPERTY VS_DEBUGGER_WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}/res")

target_include_directories(heightmap_extractor PRIVATE include)
target_include_directories(heightmap_extractor PRIVATE curl/include)

get_target_property(LIBZIP_CONF_DIR zip BINARY_DIR)
message("Using libzip configuration: ${LIBZIP_CONF_DIR}/../")
target_include_directories(heightmap_extractor PRIVATE ${LIBZIP_CONF_DIR}/../)
target_include_directories(heightmap_extractor PRIVATE libzip/lib)

target_link_libraries(heightmap_extractor PRIVATE libcurl zip)

source_group("Headers" FILES ${hpp})
source_group("Source" FILES ${cpp})

if(MSVC)
    target_compile_options(heightmap_extractor PRIVATE /W4 /WX /MD /MP /wd26812 /wd4201 /EHsc /GR)
else()
    target_compile_options(heightmap_extractor PRIVATE -Wall -Wpedantic -Wextra -Werror)
endif()