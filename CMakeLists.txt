cmake_minimum_required(VERSION 3.10)
project(wbcs VERSION 1.0.0 LANGUAGES CXX)

add_library(wbcs
    wbcs.hpp
    wbcs.cpp
)

set_target_properties(wbcs
PROPERTIES
    CXX_STANDARD 14
    PUBLIC_HEADER wbcs.hpp
)

target_include_directories(wbcs
PUBLIC
    $<INSTALL_INTERFACE:${CMAKE_INSTALL_PREFIX}/include>
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
PRIVATE
    externals/include
)

if(${CMAKE_CXX_COMPILER_ID} MATCHES GNU)
    target_compile_options(wbcs
    PUBLIC
        -std=gnu++14
    )
    target_link_libraries(wbcs
    PUBLIC
        stdc++
    )
elseif(${CMAKE_CXX_COMPILER_ID} MATCHES Clang)
    if(WIN32)
        # clang-cl is not considered for now ...
    else()
        target_compile_options(wbcs
        PUBLIC
            -stdlib=libc++
        PRIVATE
            -Wall -Wextra
        )
    endif()
elseif(MSVC)
    target_compile_options(wbcs
    PUBLIC
        /W4
    PRIVATE
        /Zc:__cplusplus
    )
endif()

if(WIN32)
    set_target_properties(wbcs
    PROPERTIES
        WINDOWS_EXPORT_ALL_SYMBOLS True
    )

    target_link_libraries(wbcs
    PUBLIC
        ws2_32
    )
endif()

target_link_libraries(wbcs
PUBLIC
    ${CMAKE_DL_LIBS}
)

install(FILES           wbcs.hpp
        DESTINATION     ${CMAKE_INSTALL_PREFIX}/include/${PROJECT_NAME}
)
install(TARGETS         wbcs
        EXPORT          ${PROJECT_NAME}-config
        RUNTIME         DESTINATION ${CMAKE_INSTALL_PREFIX}/bin
        LIBRARY         DESTINATION ${CMAKE_INSTALL_PREFIX}/lib
        ARCHIVE         DESTINATION ${CMAKE_INSTALL_PREFIX}/lib
        INCLUDES        DESTINATION ${CMAKE_INSTALL_PREFIX}/include
        FRAMEWORK       DESTINATION ${CMAKE_INSTALL_PREFIX}
)

install(EXPORT          ${PROJECT_NAME}-config
        DESTINATION     ${CMAKE_INSTALL_PREFIX}/share/${PROJECT_NAME}
)

include(CMakePackageConfigHelpers)
set(VERSION_FILE_PATH   ${CMAKE_BINARY_DIR}/cmake/${PROJECT_NAME}-config-version.cmake)
write_basic_package_version_file(${VERSION_FILE_PATH}
    VERSION             ${PROJECT_VERSION}
    COMPATIBILITY       SameMajorVersion
)

install(FILES           ${VERSION_FILE_PATH} 
        DESTINATION     ${CMAKE_INSTALL_PREFIX}/share/${PROJECT_NAME}
)


if(NOT BUILD_TESTING)
    return()
endif()
enable_testing()

add_executable(wbcs_test
    wbcs_test.cpp
)

set_target_properties(wbcs_test
PROPERTIES
    CXX_STANDARD 17
)

target_include_directories(wbcs_test
PRIVATE
    externals/include
)
if(${CMAKE_TOOLCHAIN_FILE} MATCHES vcpkg.cmake)
    target_include_directories(wbcs_test
    PRIVATE
        ${_VCPKG_INSTALLED_DIR}/${VCPKG_TARGET_TRIPLET}/include
    )
endif()

target_link_libraries(wbcs_test
PRIVATE
    wbcs
)

target_compile_definitions(wbcs_test
PRIVATE
    UTF8_ASSET_DIR=u8"${PROJECT_SOURCE_DIR}"
)

if(MSVC)
    target_compile_options(wbcs_test
    PRIVATE
        /std:c++latest
    )
endif()

add_test(NAME test_locale_native    COMMAND wbcs_test "")

if(WIN32)
else()
    # add_test(NAME test_locale_posix     COMMAND wbcs_test "POSIX")
endif()
