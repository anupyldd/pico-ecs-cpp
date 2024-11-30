cmake_minimum_required (VERSION 3.8)

project(pico_ecs_cpp LANGUAGES CXX C)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

####################################################

include(FetchContent)

message("Fetching pico_headers...")
FetchContent_Declare(
  picoheaders
  GIT_REPOSITORY https://github.com/empyreanx/pico_headers.git
  GIT_TAG 080269a444fc4e4577ac40d929d1134a475f0f0e
)
FetchContent_MakeAvailable(picoheaders)

####################################################

set(SRCS

src/PicoEcsCpp.h
src/Example.cpp

)

####################################################

add_executable("${PROJECT_NAME}" ${SRCS})

message("Including directories...")
target_include_directories(
	"${PROJECT_NAME}" 
	PUBLIC 
	${picoheaders_SOURCE_DIR}
)

if (CMAKE_VERSION VERSION_GREATER 3.12)
  set_property(TARGET pico_ecs_cpp PROPERTY CXX_STANDARD 11)
endif()

add_compile_options(/utf-8)