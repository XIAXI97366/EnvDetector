# For more information about using CMake with Android Studio, read the
# documentation: https://d.android.com/studio/projects/add-native-code.html.
# For more examples on how to use CMake, see https://github.com/android/ndk-samples.

# Sets the minimum CMake version required for this project.
cmake_minimum_required(VERSION 3.22.1)

# Declares the project name. The project name can be accessed via ${ PROJECT_NAME},
# Since this is the top level CMakeLists.txt, the project name is also accessible
# with ${CMAKE_PROJECT_NAME} (both CMake variables are in-sync within the top level
# build script scope).
project("xx-guard")

# Creates and names a library, sets it as either STATIC
# or SHARED, and provides the relative paths to its source code.
# You can define multiple libraries, and CMake builds them for you.
# Gradle automatically packages shared libraries with your APK.
#
# In this top level CMakeLists.txt, ${CMAKE_PROJECT_NAME} is used to define
# the target library name; in the sub-module's CMakeLists.txt, ${PROJECT_NAME}
# is preferred for the same purpose.
#
# In order to load a library into your app from Java/Kotlin, you must call
# System.loadLibrary() and pass the name of the library defined here;
# for GameActivity/NativeActivity derived applications, the same library name must be
# used in the AndroidManifest.xml file.

#message(WARNING "CMAKE_CXX_FLAGS is: ${CMAKE_CXX_FLAGS}")

#set函数， 将 CMAKE_CXX_FLAGS 属性在原有的基础上在添加 -Wno-narrowing -std=c++17
# CMAKE_CXX_FLAGS 该变量属于系统环境自带的变量用于C++
# CMAKE_C_FLAGS 该变量则是用于C
#set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-narrowing -std=c++17")

# MESSAGE 函数 用于打印当前的 CMAKE_CXX_FLAGS 变量中的数值， "XIAXI"则是tags， STATUS 则是根据当前的架构、debug或release所决定
#message(WARNING "CMAKE_CXX_FLAGS is: ${CMAKE_CXX_FLAGS}")

add_subdirectory(./util/xdl)
add_subdirectory(./util/hook_util)
enable_language(C ASM)

if(${CMAKE_ANDROID_ARCH_ABI} STREQUAL "arm64-v8a")
    #set_source_files_properties(syscall_AARCH_64.s PROPERTIES COMPILE_FLAGS "-x assembler-with-cpp")
    add_library(${CMAKE_PROJECT_NAME} SHARED
            # List C/C++ source files with relative paths to this CMakeLists.txt.
            xxGuard.cpp
            antiHook/antiFrida/antiFrida.cpp
            antiHook/antiXPosed/antiXPosed.cpp
            antiHook/antiPlt/antiPlt.cpp
            antiHook/antiHook.cpp
            antiDebug/antiDebug.cpp
            antiDebug/guardPorcess/guardPorcess.cpp
            util/helper/helper.cpp
            romEnv/romEnv.cpp
            romEnv/rootOfTrust.cpp
            crypto/SM4/sm4.c
            util/jniRefInvoke/jniRefInvoke.cpp
            mem/memScan.cpp
            util/elf_util/elf_util.cpp
            mem/memCrc.cpp
            linkerInfo/linkerInfo.cpp
            antiInject/checkMaps.cpp
            util/inlineASM/raw_syscall/syscall_AARCH_64.s)
elseif(${CMAKE_ANDROID_ARCH_ABI} STREQUAL "armeabi-v7a")
    #set_source_files_properties(syscall_ARM_32.s PROPERTIES COMPILE_FLAGS "-x assembler-with-cpp")
    add_library(${CMAKE_PROJECT_NAME} SHARED
            # List C/C++ source files with relative paths to this CMakeLists.txt.
            xxGuard.cpp
            antiHook/antiFrida/antiFrida.cpp
            antiHook/antiXPosed/antiXPosed.cpp
            antiHook/antiPlt/antiPlt.cpp
            antiHook/antiHook.cpp
            antiDebug/antiDebug.cpp
            antiDebug/guardPorcess/guardPorcess.cpp
            util/helper/helper.cpp
            romEnv/romEnv.cpp
            romEnv/rootOfTrust.cpp
            crypto/SM4/sm4.c
            util/jniRefInvoke/jniRefInvoke.cpp
            mem/memScan.cpp
            util/elf_util/elf_util.cpp
            mem/memCrc.cpp
            linkerInfo/linkerInfo.cpp
            antiInject/checkMaps.cpp
            util/inlineASM/raw_syscall/syscall_ARM_32.s)
elseif(${CMAKE_ANDROID_ARCH_ABI} STREQUAL "x86")
    add_library(${CMAKE_PROJECT_NAME} SHARED
            # List C/C++ source files with relative paths to this CMakeLists.txt.
            xxGuard.cpp
            antiHook/antiFrida/antiFrida.cpp
            antiHook/antiXPosed/antiXPosed.cpp
            antiHook/antiPlt/antiPlt.cpp
            antiHook/antiHook.cpp
            antiDebug/antiDebug.cpp
            antiDebug/guardPorcess/guardPorcess.cpp
            util/helper/helper.cpp
            romEnv/romEnv.cpp
            romEnv/rootOfTrust.cpp
            crypto/SM4/sm4.c
            util/jniRefInvoke/jniRefInvoke.cpp
            mem/memScan.cpp
            util/elf_util/elf_util.cpp
            mem/memCrc.cpp
            linkerInfo/linkerInfo.cpp
            antiInject/checkMaps.cpp)
elseif(${CMAKE_ANDROID_ARCH_ABI} STREQUAL "x86_64")
    add_library(${CMAKE_PROJECT_NAME} SHARED
            # List C/C++ source files with relative paths to this CMakeLists.txt.
            xxGuard.cpp
            antiHook/antiFrida/antiFrida.cpp
            antiHook/antiXPosed/antiXPosed.cpp
            antiHook/antiPlt/antiPlt.cpp
            antiHook/antiHook.cpp
            antiDebug/antiDebug.cpp
            antiDebug/guardPorcess/guardPorcess.cpp
            util/helper/helper.cpp
            romEnv/romEnv.cpp
            romEnv/rootOfTrust.cpp
            crypto/SM4/sm4.c
            util/jniRefInvoke/jniRefInvoke.cpp
            mem/memScan.cpp
            util/elf_util/elf_util.cpp
            mem/memCrc.cpp
            linkerInfo/linkerInfo.cpp
            antiInject/checkMaps.cpp)
endif()

# Specifies libraries CMake should link to your target library. You
# can link libraries from various origins, such as libraries defined in this
# build script, prebuilt third-party libraries, or Android system libraries.
target_link_libraries(${CMAKE_PROJECT_NAME}
        # List libraries link to the target library
        android
        xdl
        runtimehook
        log)

target_link_libraries(
        ${CMAKE_PROJECT_NAME}
)