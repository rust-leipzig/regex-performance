include(ExternalProject)

# @param[in]    NAME        Name of the engine. Based on the uppercase string of the name a variable
#                           `INCLUDE_<NAME> is set for further processing.
# @param[in]    LIB_NAME    Library name which is used to locate the library on the system.
# @param[in]    URL         Used URL to download the sources. Git repositories have to contain `.git` for identification.
# @param[in]    ARGN        All preceeding arguments are forwareded to `CMAKE_ARGS`, if external project uses cmake
#                           for configuration, or `CONFIGURE_COMMAND`.
function(AddExternalProject NAME LIB_NAME URL TAG)
    string(TOUPPER ${NAME} UP_NAME)

    set(INCLUDE_${UP_NAME} "system" CACHE STRING "Use ${NAME} library form local built, system or disable usage.")
    set_property(CACHE INCLUDE_${UP_NAME} PROPERTY STRINGS "local" "system" "disabled")

    message("-- Include ${NAME}: ${INCLUDE_${UP_NAME}}")

    if(INCLUDE_${UP_NAME} MATCHES "local")
        message("   > Building ${NAME} from source: ${URL}\n")

        # git repository urls have to end with '.git'
        if(${URL} MATCHES ".git$")
            string(CONCAT ARGN_STRING ${ARGN})
            if(${ARGN_STRING} MATCHES "CMAKE")
                ExternalProject_Add(
                    lib${NAME}
                    GIT_REPOSITORY ${URL}
                    GIT_TAG ${TAG}
                    GIT_EXECUTABLE /usr/bin/git
                    PREFIX ${CMAKE_CURRENT_SOURCE_DIR}
                    SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/${NAME}
                    TMP_DIR ${PROJECT_BINARY_DIR}/${NAME}-tmp
                    STAMP_DIR ${PROJECT_BINARY_DIR}/${NAME}-stamp
                    BINARY_DIR ${PROJECT_BINARY_DIR}/${NAME}-build
                    DOWNLOAD_DIR ${PROJECT_BINARY_DIR}/${NAME}-down
                    CMAKE_ARGS ${ARGN}
                )
            else()
                ExternalProject_Add(
                    lib${NAME}
                    GIT_REPOSITORY ${URL}
                    GIT_EXECUTABLE /usr/bin/git
                    PREFIX ${CMAKE_CURRENT_SOURCE_DIR}
                    SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/${NAME}
                    TMP_DIR ${PROJECT_BINARY_DIR}/${NAME}-tmp
                    STAMP_DIR ${PROJECT_BINARY_DIR}/${NAME}-stamp
                    BINARY_DIR ${PROJECT_BINARY_DIR}/${NAME}-build
                    DOWNLOAD_DIR ${PROJECT_BINARY_DIR}/${NAME}-down
                    CONFIGURE_COMMAND ${ARGN}
                )
            endif()
        else()
            ExternalProject_Add(
                lib${NAME}
                URL ${URL}
                PREFIX ${CMAKE_CURRENT_SOURCE_DIR}
                SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/${NAME}
                TMP_DIR ${PROJECT_BINARY_DIR}/${NAME}-tmp
                STAMP_DIR ${PROJECT_BINARY_DIR}/${NAME}-stamp
                BINARY_DIR ${PROJECT_BINARY_DIR}/${NAME}-build
                DOWNLOAD_DIR ${PROJECT_BINARY_DIR}/${NAME}-down
                CONFIGURE_COMMAND ${ARGN}
            )
        endif()
    elseif(INCLUDE_${UP_NAME} MATCHES "system")
        find_library(LIB_${UP_NAME} ${LIB_NAME} ${CMAKE_CURRENT_SOURCE_DIR}/local/lib)
        if(NOT LIB_${UP_NAME})
            message(FATAL_ERROR "   > Library '${LIB_NAME}' not found on system.\n")
        else()
            message("  > Found ${NAME} library: ${LIB_${UP_NAME}}\n")
        endif()
    else()
        message("")
    endif()
endfunction()



# PCRE2
set(INCLUDE_PCRE2 "system" CACHE STRING "Use pcre2 library form local built, system or disable usage.")
set_property(CACHE INCLUDE_PCRE2 PROPERTY STRINGS "local" "system" "disabled")
message("-- Include pcre2: ${INCLUDE_PCRE2}")
if(INCLUDE_PCRE2 MATCHES "system")
    find_library(LIB_PCRE2 pcre2-8 ${CMAKE_CURRENT_SOURCE_DIR}/local/lib)
    if(NOT LIB_PCRE2)
        message(FATAL_ERROR "   > Library 'pcre2-8' not found on system.\n")
    else()
        message("  > Found pcre2 library: ${LIB_PCRE2}\n")
    endif()
endif()

# Oniguruma
set(INCLUDE_ONIGURUMA "system" CACHE STRING "Use oniguruma library form local built, system or disable usage.")
set_property(CACHE INCLUDE_ONIGURUMA PROPERTY STRINGS "local" "system" "disabled")
message("-- Include oniguruma: ${INCLUDE_ONIGURUMA}")
if(INCLUDE_ONIGURUMA MATCHES "system")
    find_library(LIB_ONIGURUMA onig ${CMAKE_CURRENT_SOURCE_DIR}/local/lib)
    if(NOT LIB_ONIGURUMA)
        message(FATAL_ERROR "   > Library 'onig' not found on system.\n")
    else()
        message("  > Found oniguruma library: ${LIB_ONIGURUMA}\n")
    endif()
endif()

# TRE
set(INCLUDE_TRE "system" CACHE STRING "Use tre library form local built, system or disable usage.")
set_property(CACHE INCLUDE_TRE PROPERTY STRINGS "local" "system" "disabled")
message("-- Include tre: ${INCLUDE_TRE}")
if(INCLUDE_TRE MATCHES "system")
    find_library(LIB_TRE tre ${CMAKE_CURRENT_SOURCE_DIR}/local/lib)
    if(NOT LIB_TRE)
        message(FATAL_ERROR "   > Library 'tre' not found on system.\n")
    else()
        message("  > Found tre library: ${LIB_TRE}\n")
    endif()
endif()

# RE2
set(INCLUDE_RE2 "system" CACHE STRING "Use re2 library form local built, system or disable usage.")
set_property(CACHE INCLUDE_RE2 PROPERTY STRINGS "local" "system" "disabled")
message("-- Include re2: ${INCLUDE_RE2}")
if(INCLUDE_RE2 MATCHES "system")
    find_library(LIB_RE2 re2 ${CMAKE_CURRENT_SOURCE_DIR}/local/lib)
    if(NOT LIB_RE2)
        message(FATAL_ERROR "   > Library 're2' not found on system.\n")
    else()
        message("  > Found re2 library: ${LIB_RE2}\n")
    endif()
endif()

# CTRE (header-only)
set(INCLUDE_CTRE "system" CACHE STRING "Use ctre library form local built, system or disable usage.")
set_property(CACHE INCLUDE_CTRE PROPERTY STRINGS "local" "system" "disabled")
message("-- Include ctre: ${INCLUDE_CTRE}")

set(INCLUDE_BOOST "system" CACHE STRING "Use boost::regex library form local built, system or disable usage.")
set_property(CACHE INCLUDE_BOOST PROPERTY STRINGS "local" "system" "disabled")
message("-- Include boost: ${INCLUDE_BOOST}")
if(INCLUDE_BOOST MATCHES "system")
    find_library(LIB_BOOST boost_regex ${CMAKE_CURRENT_SOURCE_DIR}/local/lib)
    if(NOT LIB_BOOST)
        message(FATAL_ERROR "   > Library 'boost_regex' not found on system.\n")
    else()
        message("  > Found boost library: ${LIB_BOOST}\n")
    endif()
endif()

# YARA
set(INCLUDE_YARA "system" CACHE STRING "Use yara library form local built, system or disable usage.")
set_property(CACHE INCLUDE_YARA PROPERTY STRINGS "local" "system" "disabled")
message("-- Include yara: ${INCLUDE_YARA}")
if(INCLUDE_YARA MATCHES "system")
    find_library(LIB_YARA yara ${CMAKE_CURRENT_SOURCE_DIR}/local/lib)
    if(NOT LIB_YARA)
        message(FATAL_ERROR "   > Library 'yara' not found on system.\n")
    else()
        message("  > Found yara library: ${LIB_YARA}\n")
    endif()
endif()

# Hyperscan
set(INCLUDE_HYPERSCAN "system" CACHE STRING "Use hyperscan library form local built, system or disable usage.")
set_property(CACHE INCLUDE_HYPERSCAN PROPERTY STRINGS "local" "system" "disabled")
message("-- Include hyperscan: ${INCLUDE_HYPERSCAN}")
if(INCLUDE_HYPERSCAN MATCHES "system")
    find_library(LIB_HYPERSCAN hs ${CMAKE_CURRENT_SOURCE_DIR}/local/lib)
    if(NOT LIB_HYPERSCAN)
        message(FATAL_ERROR "   > Library 'hs' not found on system.\n")
    else()
        message("  > Found hyperscan library: ${LIB_HYPERSCAN}\n")
    endif()
endif()

# c++ standard
set(INCLUDE_CPPSTD "system" CACHE STRING "Use cppstd library form local built, system or disable usage.")
set_property(CACHE INCLUDE_CPPSTD PROPERTY STRINGS "local" "system" "disabled")
message("-- Include cppstd: ${INCLUDE_CPPSTD}")




