
list(APPEND COMMON_PUBLIC_HEADERS
        "include/common/platform.h"
        "include/common/config.h"
        "include/common/assert.h"
        "include/common/non_copiable.h"
        "include/common/logging.h"
        )

list(APPEND COMMON_SRC
        "src/assert.cpp"
        "src/logging.cpp"
        ${COMMON_PUBLIC_HEADERS}
        )

list(APPEND COMMON_PUBLIC_INCLUDE_DIRS
        ${CMAKE_CURRENT_SOURCE_DIR}/include
        )

find_package(Threads REQUIRED)

set(COMMON_PUBLIC_LIBRARIES spdlog Boost::boost Threads::Threads)
if(WIN32)
list(APPEND COMMON_PUBLIC_LIBRARIES dbghelp)
endif(WIN32)

asap_library(
        TARGET
        asap_common
        SOURCES
        ${COMMON_SRC}
        EXPORT_NAME
        common
        PUBLIC_INCLUDE_DIRS
        ${COMMON_PUBLIC_INCLUDE_DIRS}
        PUBLIC_LIBRARIES
        ${COMMON_PUBLIC_LIBRARIES}
)
set_tidy_target_properties(asap_common)

set_cppcheck_command()

add_subdirectory(test)

configure_doxyfile(CommonLib
                   "\"Common Module\""
                   "\"A module to provide the absolute minimum required building blocks for the other modules such as portability, logging, assertions, etc...\""
                   "${CMAKE_CURRENT_SOURCE_DIR}/src ${CMAKE_CURRENT_SOURCE_DIR}/include")
add_doxygen_target(CommonLib)
