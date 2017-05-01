cmake_minimum_required(VERSION 3.4.1)

add_library( testlib SHARED
    src/main/cpp/CppSurfaceView.cpp
    src/main/cpp/CppPng.cpp
    src/main/cpp/png.c
    src/main/cpp/pngerror.c
    src/main/cpp/pngget.c
    src/main/cpp/pngmem.c
    src/main/cpp/pngpread.c
    src/main/cpp/pngread.c
    src/main/cpp/pngrio.c
    src/main/cpp/pngrtran.c
    src/main/cpp/pngrutil.c
    src/main/cpp/pngset.c
    src/main/cpp/pngtest.c
    src/main/cpp/pngtrans.c
    src/main/cpp/pngwio.c
    src/main/cpp/pngwrite.c
    src/main/cpp/pngwtran.c
    src/main/cpp/pngwutil.c
)

find_library( log-lib log )

target_link_libraries( testlib ${log-lib} android EGL GLESv2 z )
