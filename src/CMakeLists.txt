add_subdirectory(rust)

# configure files
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/version.h.in ${CMAKE_CURRENT_SOURCE_DIR}/version.h)

set(REGEX_SOURCES
    main.c
    onig.c
    hyperscan.c
    pcre2.c
    rust.c
    tre.c
    re2.cpp
)

include_directories(
    ${PROJECT_SOURCE_DIR}/vendor/local/include
    ${CMAKE_CURRENT_SOURCE_DIR}/rust/include
)

link_directories(${PROJECT_SOURCE_DIR}/vendor/local/lib)

if(CMAKE_BUILD_TYPE MATCHES "^Release")
    link_directories(${PROJECT_BINARY_DIR}/rregex/release)
else()
    link_directories(${PROJECT_BINARY_DIR}/rregex/debug)
endif()

add_executable(regex_perf ${REGEX_SOURCES})

add_dependencies(regex_perf librregex)

target_link_libraries(regex_perf rregex hs onig re2 tre pcre2-8 pthread dl)
