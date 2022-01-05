include(ExternalProject)

# @param[in]    NAME        Name of the engine. Based on the uppercase string of the name a variable
#                           `INCLUDE_<NAME> is set for further processing.
# @param[in]    LIB_NAME    Library name which is used to locate the library on the system.
# @param[in]    URL         Used URL to download the sources. Git repositories have to contain `.git` for identification.
# @param[in]    ARGN        All preceeding arguments are forwareded to `CMAKE_ARGS`, if external project uses cmake
#                           for configuration, or `CONFIGURE_COMMAND`.
function(AddExternalProject NAME LIB_NAME URL TAG)
    string(TOUPPER ${NAME} UP_NAME)

    set(INCLUDE_${UP_NAME} "local" CACHE STRING "Use ${NAME} library form local built, system or disable usage.")
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

# hyperscan
AddExternalProject(
    "hyperscan"
    "hs"
    "https://github.com/01org/hyperscan.git"
    "master"
    -DCMAKE_BUILD_TYPE=Release -DFAT_RUNTIME=OFF -DCMAKE_INSTALL_PREFIX=${CMAKE_CURRENT_SOURCE_DIR}/local
)

# oniguruma
AddExternalProject(
    "oniguruma"
    "onig"
    "https://github.com/kkos/oniguruma.git"
    "master"
    -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX=${CMAKE_CURRENT_SOURCE_DIR}/local
)

# re2
AddExternalProject(
    "re2"
    "re2"
    "https://github.com/google/re2.git"
    "main"
    -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX=${CMAKE_CURRENT_SOURCE_DIR}/local
)

# tre
AddExternalProject(
    "tre"
    "tre"
    "https://github.com/laurikari/tre.git"
    "master"
    cd ${CMAKE_CURRENT_SOURCE_DIR}/tre/ && ./utils/autogen.sh && cd ${PROJECT_BINARY_DIR}/tre-build && ${CMAKE_CURRENT_SOURCE_DIR}/tre/configure --prefix=${CMAKE_CURRENT_SOURCE_DIR}/local
)

# pcre2
AddExternalProject(
    "pcre2"
    "pcre2-8"
    "https://github.com/PhilipHazel/pcre2.git"
    "master"
    -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX=${CMAKE_CURRENT_SOURCE_DIR}/local -DPCRE2_SUPPORT_JIT=ON
)

# c++ standard
set(INCLUDE_CPPSTD "system" CACHE STRING "Use cppstd library form local built, system or disable usage.")
set_property(CACHE INCLUDE_CPPSTD PROPERTY STRINGS "local" "system" "disabled")
message("-- Include cppstd: ${INCLUDE_CPPSTD}")

# ctre
# 
AddExternalProject(
    "ctre"
    "ctre"
    "https://github.com/hanickadot/compile-time-regular-expressions.git"
    "master"
    -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX=${CMAKE_CURRENT_SOURCE_DIR}/local 
)

# boost - I'm not going to build boost here
set(INCLUDE_BOOST "system" CACHE STRING "Use boost::regex library form local built, system or disable usage.")
set_property(CACHE INCLUDE_BOOST PROPERTY STRINGS "local" "system" "disabled")
message("-- Include boost: ${INCLUDE_BOOST}")
