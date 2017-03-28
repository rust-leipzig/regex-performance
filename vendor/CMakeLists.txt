include(ExternalProject)

# @param[in]    NAME        Name of the engine. Based on the uppercase string of the name a variable
#                           `INCLUDE_<NAME> is set for further processing.
# @param[in]    LIB_NAME    Library name which is used to locate the library on the system.
# @param[in]    URL         Used URL to download the sources. Git repositories have to contain `.git` for identification.
# @param[in]    ARGN        All preceeding arguments are forwareded to `CMAKE_ARGS`, if external project uses cmake
#                           for configuration, or `CONFIGURE_COMMAND`.
function(AddExternalProject NAME LIB_NAME URL)
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
    -DCMAKE_BUILD_TYPE=Release -DFAT_RUNTIME=OFF -DCMAKE_INSTALL_PREFIX=${CMAKE_CURRENT_SOURCE_DIR}/local
)

# oniguruma
AddExternalProject(
    "oniguruma"
    "onig"
    "https://github.com/kkos/oniguruma.git"
    -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX=${CMAKE_CURRENT_SOURCE_DIR}/local
)

# re2
AddExternalProject(
    "re2"
    "re2"
    "https://github.com/google/re2.git"
    -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX=${CMAKE_CURRENT_SOURCE_DIR}/local
)

# tre
AddExternalProject(
    "tre"
    "tre"
    "https://github.com/laurikari/tre.git"
    cd ${CMAKE_CURRENT_SOURCE_DIR}/tre/ && ./utils/autogen.sh && cd ${PROJECT_BINARY_DIR}/tre-build && ${CMAKE_CURRENT_SOURCE_DIR}/tre/configure --prefix=${CMAKE_CURRENT_SOURCE_DIR}/local
)

# pcre2
AddExternalProject(
    "pcre2"
    "pcre2-8"
    "https://ftp.pcre.org/pub/pcre/pcre2-10.23.tar.gz"
    ${CMAKE_CURRENT_SOURCE_DIR}/pcre2/configure --prefix=${CMAKE_CURRENT_SOURCE_DIR}/local --enable-unicode --enable-jit
)
