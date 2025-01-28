cmake_minimum_required(VERSION 3.10)
project(MyProject)

# Set C++ standard (optional)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} /SUBSYSTEM:WINDOWS")

set(ASSIMP_INCLUDE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/src/third_party/assimp/include")
set(ASSIMP_LIBRARY "${CMAKE_CURRENT_SOURCE_DIR}/src/third_party/build/lib/Release/assimp-vc143-mt.lib")

# Add your source files here
add_executable(MyProject
    src/main.cpp
)

target_include_directories(MyProject PRIVATE "${ASSIMP_INCLUDE_DIR}")

target_link_libraries(MyProject PRIVATE
    d3d11
    dxgi
    dxguid
    d3dcompiler
    "${ASSIMP_LIBRARY}"
)
