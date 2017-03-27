add_subdirectory(rust)

# configure files
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/version.h.in ${CMAKE_CURRENT_SOURCE_DIR}/version.h)

set(REGEX_SOURCES
    main.c
    rust.c
)

if(NOT ${INCLUDE_PCRE2} MATCHES "disabled")
    add_definitions(-DINCLUDE_PCRE2)
    set(REGEX_SOURCES ${REGEX_SOURCES} pcre2.c)
    set(REGEX_ENGINES ${REGEX_ENGINES} pcre2-8)
endif()

if(NOT ${INCLUDE_HYPERSCAN} MATCHES "disabled")
    add_definitions(-DINCLUDE_HYPERSCAN)
    set(REGEX_SOURCES ${REGEX_SOURCES} hyperscan.c)
    set(REGEX_ENGINES ${REGEX_ENGINES} hs)
endif()

if(NOT ${INCLUDE_ONIGURUMA} MATCHES "disabled")
    add_definitions(-DINCLUDE_ONIGURUMA)
    set(REGEX_SOURCES ${REGEX_SOURCES} onig.c)
    set(REGEX_ENGINES ${REGEX_ENGINES} onig)
endif()

if(NOT ${INCLUDE_TRE} MATCHES "disabled")
    add_definitions(-DINCLUDE_TRE)
    set(REGEX_SOURCES ${REGEX_SOURCES} tre.c)
    set(REGEX_ENGINES ${REGEX_ENGINES} tre)
endif()

if(NOT ${INCLUDE_RE2} MATCHES "disabled")
    add_definitions(-DINCLUDE_RE2)
    set(REGEX_SOURCES ${REGEX_SOURCES} re2.cpp)
    set(REGEX_ENGINES ${REGEX_ENGINES} re2)
endif()

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

target_link_libraries(regex_perf rregex ${REGEX_ENGINES} pthread dl)
