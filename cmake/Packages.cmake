include(FetchContent)
#include(FindPkgConfig)

if(ESL_DEPENDENCIES_USE_CONAN)
    message(STATUS "Using Conan")
    include(${CMAKE_BINARY_DIR}/conan/conan_toolchain.cmake)
endif()

if(ESL_DEPENDENCIES_USE_VCPKG)
    message(STATUS "Using VCPKG")
    if(WIN32)
        set(USER_HOME_DIRECTORY $ENV{USERPROFILE})
    else()
        set(USER_HOME_DIRECTORY $ENV{HOME})
    endif()
    message(STATUS "User Home Directory: ${USER_HOME_DIRECTORY}")
    include(${USER_HOME_DIRECTORY}/opt/vcpkg/scripts/buildsystems/vcpkg.cmake)
endif()

if(ESL_DEPENDENCIES_USE_PKGCONFIG)
    find_package(PkgConfig QUIET)
endif()

function(find_package_esa)
    # Default, try 'find_package'. VCPKG or Conan may be used, if enabled
    if(NOT esa_FOUND)
        message(STATUS "Try to find esa by find_package")
        find_package(esa QUIET)
        if(esa_FOUND)
            message(STATUS "esa has been found by using find_package")
        endif()
    endif()

    if(NOT esa_FOUND)
        message(STATUS "Try to find esa by FetchContent")
        FetchContent_Declare(
            esa
            GIT_REPOSITORY https://github.com/SLukasDE/esa
            #GIT_TAG master
            GIT_SHALLOW TRUE
            OVERRIDE_FIND_PACKAGE # 'find_package(...)' will call 'FetchContent_MakeAvailable(...)'
        )
        find_package(esa QUIET)
        if(esa_FOUND)
            message(STATUS "esa has been found by using FetchContent")
        endif()
    endif()

    if(NOT esa_FOUND)
        message(FATAL_ERROR "esa NOT found")
    endif()
endfunction()

function(find_package_esl)
    # Default, try 'find_package'. VCPKG or Conan may be used, if enabled
    if(NOT esl_FOUND)
        message(STATUS "Try to find esl by find_package")
        find_package(esl QUIET)
        if(esl_FOUND)
            message(STATUS "esl has been found by using find_package")
        endif()
    endif()

    if(NOT esl_FOUND)
        message(STATUS "Try to find esl by FetchContent")
        FetchContent_Declare(
            esl
            GIT_REPOSITORY https://github.com/SLukasDE/esl
            #GIT_TAG master
            GIT_SHALLOW TRUE
            OVERRIDE_FIND_PACKAGE # 'find_package(...)' will call 'FetchContent_MakeAvailable(...)'
        )
        find_package(esl QUIET)
        if(esl_FOUND)
            message(STATUS "esl has been found by using FetchContent")
        endif()
    endif()

    if(NOT esl_FOUND)
        message(FATAL_ERROR "esl NOT found")
    endif()
endfunction()

function(find_package_CURL) # CURL::libcurl
    if(BUILD_SHARED_LIBS)
        set(CURL_USE_STATIC_LIBS FALSE)
    else(BUILD_SHARED_LIBS)
        set(CURL_USE_STATIC_LIBS TRUE)
    endif(BUILD_SHARED_LIBS)
	
    # Default, try 'find_package'. VCPKG or Conan may be used, if enabled
    if(NOT CURL_FOUND)
        message(STATUS "Try to find CURL by find_package")
        #find_package(CURL QUIET)
        if(CURL_FOUND)
            message(STATUS "CURL has been found by using find_package")
        endif()
    endif()
	
    if(NOT CURL_FOUND)
        message(STATUS "Try to find CURL by FetchContent")
        
        # https://github.com/curl/curl/blob/master/lib/curl_config.h.cmake
        set(BUILD_CURL_EXE      OFF)
        # Only FILE and HTTP(S) support required
        set(CURL_ENABLE_SSL     TRUE)
        set(CURL_DISABLE_FTP    TRUE)
        set(CURL_DISABLE_LDAP   TRUE)
        set(CURL_DISABLE_POP3   TRUE)
        set(CURL_DISABLE_IMAP   TRUE)
        set(CURL_DISABLE_SMB    TRUE)
        set(CURL_DISABLE_SMTP   TRUE)
        set(CURL_DISABLE_RTSP   TRUE)
        set(CURL_DISABLE_MQTT   TRUE)
        set(CURL_DISABLE_TELNET TRUE)
        set(CURL_DISABLE_TFTP   TRUE)
        set(CURL_DISABLE_DICT   TRUE)
        set(CURL_DISABLE_GOPHER TRUE)

		#[[
        if(WIN32)
            add_definitions("-DCURL_STATICLIB")
            set(ENABLE_UNICODE ON CACHE BOOL INTERNAL "Set to ON to use the Unicode version of the Windows API functions")
        endif()
        ]]

        #[[ 
        FetchContent_Declare(curl
            GIT_REPOSITORY https://github.com/curl/curl.git
            GIT_TAG  curl-8_5_0
            GIT_SHALLOW TRUE
            OVERRIDE_FIND_PACKAGE # 'find_package(...)' will call 'FetchContent_MakeAvailable(...)'
            #SUBBUILD_DIR "${DEPS_ARTIFACTS}/build/curl/"
            #TMP_DIR "${DEPS_ARTIFACTS}/tmp/curl/"
            #STAMP_DIR "${DEPS_ARTIFACTS}/stamp/curl/"
            #LOG_DIR "${DEPS_ARTIFACTS}/log/curl/"
            #BINARY_DIR "${DEPS_ARTIFACTS}/deps_builds/curl"
        )
        ]]

        FetchContent_Declare(
            curl
            URL https://curl.se/download/curl-8.5.0.tar.gz
            DOWNLOAD_EXTRACT_TIMESTAMP true
            OVERRIDE_FIND_PACKAGE # 'find_package(...)' will call 'FetchContent_MakeAvailable(...)'
        )
        
        find_package(CURL QUIET)
        if(CURL_FOUND)
            message(STATUS "CURL has been found by using FetchContent")
        endif()
    endif()
	
    
    if(NOT CURL_FOUND)
        message(FATAL_ERROR "CURL not found")
    endif()

    if(NOT TARGET CURL::libcurl)
        message(FATAL_ERROR "TARGET CURL::libcurl does not exists")
    endif()
endfunction()
