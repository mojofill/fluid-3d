@echo off

echo Compiling shaders ...

slangc src\assets\shaders\vert.slang -target spirv -profile spirv_1_5 -entry main -stage vertex -o src\assets\spvs\vert.spv
slangc src\assets\shaders\frag.slang -target spirv -profile spirv_1_5 -entry main -stage fragment -o src\assets\spvs\frag.spv
slangc src\assets\shaders\raymarchDensity.slang -target spirv -profile spirv_1_5 -entry main -stage compute -o src\assets\spvs\raymarchDensity.spv
slangc src\assets\shaders\utils.slang -o src\assets\shaders\utils.slang-module
slangc src\assets\shaders\initialization.slang -target spirv -profile spirv_1_5 -entry main -stage compute -o src\assets\spvs\initialization.spv
slangc src\assets\shaders\applyExternalForces.slang -target spirv -profile spirv_1_5 -entry main -stage compute -o src\assets\spvs\applyExternalForces.spv
slangc src\assets\shaders\advectVelocities.slang -target spirv -profile spirv_1_5 -entry main -stage compute -o src\assets\spvs\advectVelocities.spv
slangc src\assets\shaders\solvePressure.slang -target spirv -profile spirv_1_5 -entry main -stage compute -o src\assets\spvs\solvePressure.spv
slangc src\assets\shaders\applyPressureForce.slang -target spirv -profile spirv_1_5 -entry main -stage compute -o src\assets\spvs\applyPressureForce.spv
slangc src\assets\shaders\advectSmoke.slang -target spirv -profile spirv_1_5 -entry main -stage compute -o src\assets\spvs\advectSmoke.spv
slangc src\assets\shaders\persistData.slang -target spirv -profile spirv_1_5 -entry main -stage compute -o src\assets\spvs\persistData.spv

echo Configuring CMake...
cmake -S . -B build ^
    -G "MinGW Makefiles" ^
    -DCMAKE_CXX_COMPILER=C:/msys64/ucrt64/bin/g++.exe ^
    -DCMAKE_MAKE_PROGRAM=C:/msys64/ucrt64/bin/mingw32-make.exe ^
    -DCMAKE_BUILD_TYPE=Release ^
    -Dglfw3_DIR=C:/glfw-mingw/lib/cmake/glfw3

echo Building...
cmake --build build

echo Running...
build\vk_app.exe
