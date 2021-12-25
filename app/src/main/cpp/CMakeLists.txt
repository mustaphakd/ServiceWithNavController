cmake_minimum_required(VERSION 3.4.1)


#set (SRCS Hello.c File2.c File3.c)
file (GLOB CPP_FILES impl/stringHelper.cpp impl/*.cpp)

file (GLOB H_FILES includes/stringHelper.h includes/*.h)

file (GLOB FS_FILES includes/fs/*.hpp)

file (GLOB ANDRD_FILES includes/android/*.h)

file (GLOB MRS_H_FILES includes/morris/*.h)
file (GLOB MRS_FILES includes/morris/*.c)


set (SOURCE_FILES ${FS_FILES} ${MRS_FILES} ${MRS_H_FILES} ${H_FILES}  ${CPP_FILES} )


include_directories ( includes )

add_library( # Specifies the name of the library. libnative-lib.so
             native-lib

             # Sets the library as a shared library.  SHARED
             SHARED

            "impl/entry.cpp"

             # Provides a relative path to your source file(s).
             ${SOURCE_FILES} )
set_target_properties(native-lib PROPERTIES LINKER_LANGUAGE CXX)
#command to your CMake build script to locate an NDK library and store its path as a variable.
find_library( # Defines the name of the path variable that stores the
              # location of the NDK library.
              log-lib

              # Specifies the name of the NDK library that
              # CMake needs to locate.
              log )

find_library( # Defines the name of the path variable that stores the
        # location of the NDK library.
        androidnaa-lib

        # Specifies the name of the NDK library that
        # CMake needs to locate. libandroid
        android )

# libcamera2ndk libmediandk Oboe libandroid m
# Links your native library against one or more other native libraries.
target_link_libraries( # Specifies the target library.
                       native-lib

                       # Links the log library to the target library.
                       ${log-lib} ${androidnaa-lib})