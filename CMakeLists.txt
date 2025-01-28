cmake_minimum_required(VERSION 3.10)
project(MyProject)

# Set C++ standard (optional)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} /SUBSYSTEM:WINDOWS")

# Add your source files here
add_executable(MyProject
    src/main.cpp
    # add other .cpp files if needed
)

# Link against DirectX libraries (for Direct3D 11, for example):
target_link_libraries(MyProject PRIVATE
    d3d11
    dxgi
    dxguid
    # Add d3dcompiler, dxcompiler, or others if you need them
)

# If you have separate includes, you can do:
# target_include_directories(MyProject PRIVATE "path/to/includes")
