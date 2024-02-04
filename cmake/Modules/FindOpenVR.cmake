# - Try to find the OpenVR library
#
# Once done, this will define:
#
#  OPENVR_INCLUDE_DIR - the OpenVR include directory
#  OPENVR_LIBRARIES - The libraries needed to use OpenVR

if (NOT OPENVR_INCLUDE_DIR OR NOT OPENVR_LIBRARIES)
    set(LIB_SEARCH_PATHS
        ~/Library/Frameworks
        /Library/Frameworks
        /usr/lib
        /usr/lib64
        /usr/local/lib
        /usr/local/lib64
        $ENV{OPENVRROOT}/lib/linux64
        $ENV{OPENVR_ROOT}/lib/linux64
        $ENV{OPENVR_DIR}/lib/linux64
    )

    FIND_PATH(OPENVR_INCLUDE_DIR openvr.h
        /usr/include
        /usr/local/include
        $ENV{OPENVRROOT}/headers
        $ENV{OPENVR_ROOT}/headers
        $ENV{OPENVR_DIR}/headers
        DOC "Include path for OpenVR"
    )

    if (WIN32)
        if (CMAKE_SIZEOF_VOID_P EQUAL 8)  # 64-bit
            FIND_LIBRARY(OPENVR_LIBRARY NAMES openvr_api
                PATHS
                $ENV{OPENVR_ROOT}/lib/win64
                DOC "OpenVR library for 64-bit"
            )
        else()  # 32-bit
            FIND_LIBRARY(OPENVR_LIBRARY NAMES openvr_api
                PATHS
                $ENV{OPENVR_ROOT}/lib/win32
                DOC "OpenVR library for 32-bit"
            )
        endif()
    elseif (APPLE)
        FIND_LIBRARY(OPENVR_LIBRARY NAMES openvr_api
            PATHS
            $ENV{OPENVR_ROOT}/lib/osx32
            DOC "OpenVR library for OSX"
        )
    else()  # Assuming Linux
        FIND_LIBRARY(OPENVR_LIBRARY NAMES openvr_api
            PATHS ${LIB_SEARCH_PATHS}
            DOC "OpenVR library for Linux"
        )
    endif()

    if (OPENVR_LIBRARY)
        set(OPENVR_LIBRARIES ${OPENVR_LIBRARY})
    endif()

    MARK_AS_ADVANCED(OPENVR_INCLUDE_DIR OPENVR_LIBRARIES)
endif()

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(OpenVR DEFAULT_MSG OPENVR_INCLUDE_DIR OPENVR_LIBRARIES)