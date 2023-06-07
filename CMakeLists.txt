cmake_minimum_required(VERSION 3.7)
project(cring 
    VERSION 1.1.0
    LANGUAGES C
)

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -g -fms-extensions -D_GNU_SOURCE=1")


configure_file(manifest.h.in manifest.h)
include_directories(
    ${PROJECT_SOURCE_DIR}
    ${PROJECT_BINARY_DIR}
)


# Install
install(FILES cring.h DESTINATION "include")
install(FILES cring.c DESTINATION "include")


# Lint
set(PRETTYC_FLAGS
    --recursive
    --verbose=0
    --repository=.
    --extensions=c
    --linelength=80
    --headers=h,in
    --includeorder=standardcfirst
    --root=.
    #--quiet
    ${PROJECT_SOURCE_DIR}
)
add_custom_target(lint
    COMMAND prettyc
    ${PRETTYC_FLAGS}
)


# CPack
set(CPACK_PACKAGE_FILE_NAME "libcring-${PROJECT_VERSION}")
set(CPACK_SET_DESTDIR true)
set(CPACK_PACKAGE_NAME libcring)
set(CPACK_PACKAGE_CONTACT "Vahid Mardani <vahid.mardani@gmail.com>")
set(CPACK_GENERATOR DEB)
set(CPACK_DEBIAN_PACKAGE_DEPENDS "libclog (>= 1.1)")
include(CPack)


# Testing
if (NOT DEFINED ENV{SKIP_TESTS})
include(CTest)
list(APPEND CMAKE_CTEST_ARGUMENTS "--output-on-failure")


# Test unsigned char
add_executable(test_cring test_cring.c)
target_include_directories(test_cring PUBLIC "${PROJECT_BINARY_DIR}")
target_link_libraries(test_cring PUBLIC clog)
add_test(NAME test_cring COMMAND test_cring)
add_custom_target(profile_test_cring
    COMMAND "valgrind" 
    ${VALGRIND_FLAGS}
    $<TARGET_FILE:test_cring>
)


# Test struct
add_executable(test_struct test_struct.c)
target_include_directories(test_struct PUBLIC "${PROJECT_BINARY_DIR}")
target_link_libraries(test_struct PUBLIC clog)
add_test(NAME test_struct COMMAND test_struct)
add_custom_target(profile_test_struct
    COMMAND "valgrind" 
    ${VALGRIND_FLAGS}
    $<TARGET_FILE:test_struct>
)

endif()


# Valgrind, Profiling
add_custom_target(profile 
    DEPENDS 
    profile_test_ringbuffer
)
