#cmake_minimum_required(VERSION 3.10.2)
cmake_minimum_required(VERSION 3.4.1)
set(CMAKE_VERBOSE_MAKEFILE ON)
include(ExternalProject)

set(VERBOSE 1)

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


MESSAGE(" CMAKE_FIND_ROOT_PATH  :${CMAKE_FIND_ROOT_PATH}")
MESSAGE(" CMAKE_BUILD_TYPE  :${CMAKE_BUILD_TYPE}")
MESSAGE(" CMAKE_TOOLCHAIN_FILE  :${CMAKE_TOOLCHAIN_FILE}")
MESSAGE(" ANDROID_ABI  :${ANDROID_ABI}")
MESSAGE(" ANDROID_NDK  :${ANDROID_NDK}")
MESSAGE(" ANDROID_PLATFORM  :${ANDROID_PLATFORM}")
MESSAGE(" CMAKE_ANDROID_ARCH_ABI  :${CMAKE_ANDROID_ARCH_ABI}")
MESSAGE(" CMAKE_ANDROID_NDK  :${CMAKE_ANDROID_NDK}")



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



add_library(
        crypto
        STATIC
        IMPORTED
)


set_target_properties(
        crypto
        PROPERTIES IMPORTED_LOCATION
        # The change happens here: each of the architectures gets its own subdirectory.
        ${CMAKE_CURRENT_SOURCE_DIR}/submodules/boringssl/${ANDROID_ABI}/libcrypto.a
)

add_library(
        decrepit
        STATIC
        IMPORTED
)


set_target_properties(
        decrepit
        PROPERTIES IMPORTED_LOCATION
        # The change happens here: each of the architectures gets its own subdirectory.
        ${CMAKE_CURRENT_SOURCE_DIR}/submodules/boringssl/${ANDROID_ABI}/libdecrepit.a
)

add_library(
        ssl
        STATIC
        IMPORTED
)


set_target_properties(
        ssl
        PROPERTIES IMPORTED_LOCATION
        # The change happens here: each of the architectures gets its own subdirectory.
        ${CMAKE_CURRENT_SOURCE_DIR}/submodules/boringssl/${ANDROID_ABI}/libssl.a
)


include_directories(submodules/boringssl/include)


# libcamera2ndk libmediandk Oboe libandroid m
# Links your native library against one or more other native libraries.
target_link_libraries( # Specifies the target library.
                       native-lib

                       # Links the log library to the target library. ${JSONCPP_LIBRARIES}  jsoncpp::jsoncpp   jsoncpp_lib
                       ${log-lib} ${androidnaa-lib}  ssl crypto decrepit ) #openssl::openssl