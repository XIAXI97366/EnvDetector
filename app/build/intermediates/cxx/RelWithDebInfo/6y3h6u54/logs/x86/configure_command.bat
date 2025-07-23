@echo off
"C:\\Users\\97366\\AppData\\Local\\Android\\Sdk\\cmake\\3.22.1\\bin\\cmake.exe" ^
  "-HE:\\Custom tools\\XIAXI\\app\\src\\main\\cpp" ^
  "-DCMAKE_SYSTEM_NAME=Android" ^
  "-DCMAKE_EXPORT_COMPILE_COMMANDS=ON" ^
  "-DCMAKE_SYSTEM_VERSION=21" ^
  "-DANDROID_PLATFORM=android-21" ^
  "-DANDROID_ABI=x86" ^
  "-DCMAKE_ANDROID_ARCH_ABI=x86" ^
  "-DANDROID_NDK=E:\\Environment_Variable\\android-ndk-r26c-windows\\android-ndk-r26c" ^
  "-DCMAKE_ANDROID_NDK=E:\\Environment_Variable\\android-ndk-r26c-windows\\android-ndk-r26c" ^
  "-DCMAKE_TOOLCHAIN_FILE=E:\\Environment_Variable\\android-ndk-r26c-windows\\android-ndk-r26c\\build\\cmake\\android.toolchain.cmake" ^
  "-DCMAKE_MAKE_PROGRAM=C:\\Users\\97366\\AppData\\Local\\Android\\Sdk\\cmake\\3.22.1\\bin\\ninja.exe" ^
  "-DCMAKE_CXX_FLAGS=-std=c++11" ^
  "-DCMAKE_LIBRARY_OUTPUT_DIRECTORY=E:\\Custom tools\\XIAXI\\app\\build\\intermediates\\cxx\\RelWithDebInfo\\6y3h6u54\\obj\\x86" ^
  "-DCMAKE_RUNTIME_OUTPUT_DIRECTORY=E:\\Custom tools\\XIAXI\\app\\build\\intermediates\\cxx\\RelWithDebInfo\\6y3h6u54\\obj\\x86" ^
  "-DCMAKE_BUILD_TYPE=RelWithDebInfo" ^
  "-BE:\\Custom tools\\XIAXI\\app\\.cxx\\RelWithDebInfo\\6y3h6u54\\x86" ^
  -GNinja
