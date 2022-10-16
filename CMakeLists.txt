cmake_minimum_required(VERSION 3.23)

project(curl4esl VERSION 1.5.0
            LANGUAGES CXX)
            
#include(FetchContent)
#FetchContent_Declare(
#  catch
#  GIT_REPOSITORY https://github.com/catchorg/Catch2.git
#  GIT_TAG        v2.13.6
#)
#FetchContent_MakeAvailable(catch)


# The compiled library code is here
#add_subdirectory(src/main)
# STATIC, SHARED, MODULE, INTERFACE
add_library(curl4esl STATIC
      ./src/main/curl4esl/Plugin.cpp
      ./src/main/curl4esl/Plugin.h
      ./src/main/curl4esl/Logger.h
      ./src/main/curl4esl/com/http/client/ConnectionFactory.h
      ./src/main/curl4esl/com/http/client/Send.h
      ./src/main/curl4esl/com/http/client/Connection.h
      ./src/main/curl4esl/com/http/client/Send.cpp
      ./src/main/curl4esl/com/http/client/Connection.cpp
      ./src/main/curl4esl/com/http/client/ConnectionFactory.cpp
#      ./lib.cpp
)

target_include_directories(curl4esl PUBLIC ./src/main)
target_compile_features(curl4esl PUBLIC cxx_std_11)
set_property(TARGET curl4esl PROPERTY POSITION_INDEPENDENT_CODE ON)

find_package(esl REQUIRED)
find_package(CURL REQUIRED)

target_link_libraries(curl4esl PRIVATE esl::esl)
target_link_libraries(curl4esl PRIVATE CURL::libcurl)

install(TARGETS curl4esl
        EXPORT curl4esl-export
        LIBRARY DESTINATION lib
        ARCHIVE DESTINATION lib
)

install(EXPORT      curl4esl-export
        FILE        curl4eslTargets.cmake
        NAMESPACE   curl4esl::
        DESTINATION lib/cmake
)

install(FILES ${CMAKE_CURRENT_SOURCE_DIR}/curl4eslConfig.cmake
        DESTINATION "lib/cmake"
)




# The compiled test code is here
#add_subdirectory(src/test)
