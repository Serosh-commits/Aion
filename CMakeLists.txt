cmake_minimum_required(VERSION 3.10)
project(Aion)

find_package(LLVM REQUIRED CONFIG)

include_directories(${LLVM_INCLUDE_DIRS})
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)
add_definitions(${LLVM_DEFINITIONS})

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if(NOT LLVM_ENABLE_RTTI)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fno-rtti")
endif()
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -fsanitize=address,undefined -fno-omit-frame-pointer")

file(GLOB LIB_SOURCES "lib/*.cpp")

add_library(OptDebugger STATIC ${LIB_SOURCES})

add_executable(opt-debugger tools/opt-debugger/main.cpp)

# Confirmed built components in previous turns
llvm_map_components_to_libnames(llvm_libs Core Support IRReader Analysis ScalarOpts TransformUtils Remarks BitReader InstCombine TargetParser AggressiveInstCombine)
target_link_libraries(OptDebugger PUBLIC ${llvm_libs})
target_link_libraries(opt-debugger PRIVATE OptDebugger)
