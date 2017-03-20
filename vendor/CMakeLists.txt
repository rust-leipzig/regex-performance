include(ExternalProject)

# hyperscan
ExternalProject_Add(libhs
                    GIT_REPOSITORY https://github.com/01org/hyperscan.git
                    PREFIX ${CMAKE_CURRENT_SOURCE_DIR}
                    SOURCE_DIR hyperscan
                    TMP_DIR ${PROJECT_BINARY_DIR}/hyperscan-tmp
                    STAMP_DIR ${PROJECT_BINARY_DIR}/hyperscan-stamp
                    BINARY_DIR ${PROJECT_BINARY_DIR}/hyperscan-build
                    DOWNLOAD_DIR ${PROJECT_BINARY_DIR}/hyperscan-down
                    CMAKE_ARGS -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX=${CMAKE_CURRENT_SOURCE_DIR}/local
)

# oniguruma
ExternalProject_Add(libonig
                    GIT_REPOSITORY https://github.com/kkos/oniguruma
                    PREFIX ${CMAKE_CURRENT_SOURCE_DIR}
                    SOURCE_DIR oniguruma
                    TMP_DIR ${PROJECT_BINARY_DIR}/oniguruma-tmp
                    STAMP_DIR ${PROJECT_BINARY_DIR}/oniguruma-stamp
                    BINARY_DIR ${PROJECT_BINARY_DIR}/oniguruma-build
                    DOWNLOAD_DIR ${PROJECT_BINARY_DIR}/oniguruma-down
                    CMAKE_ARGS -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX=${CMAKE_CURRENT_SOURCE_DIR}/local
)

# re2
ExternalProject_Add(libre2
                    GIT_REPOSITORY https://github.com/google/re2.git
                    PREFIX ${CMAKE_CURRENT_SOURCE_DIR}
                    SOURCE_DIR re2
                    TMP_DIR ${PROJECT_BINARY_DIR}/re2-tmp
                    STAMP_DIR ${PROJECT_BINARY_DIR}/re2-stamp
                    BINARY_DIR ${PROJECT_BINARY_DIR}/re2-build
                    DOWNLOAD_DIR ${PROJECT_BINARY_DIR}/re2-down
                    CMAKE_ARGS -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX=${CMAKE_CURRENT_SOURCE_DIR}/local
)

# tre
ExternalProject_Add(libtre
                    GIT_REPOSITORY https://github.com/laurikari/tre.git
                    PREFIX ${CMAKE_CURRENT_SOURCE_DIR}
                    SOURCE_DIR tre
                    TMP_DIR ${PROJECT_BINARY_DIR}/tre-tmp
                    STAMP_DIR ${PROJECT_BINARY_DIR}/tre-stamp
                    BINARY_DIR ${PROJECT_BINARY_DIR}/tre-build
                    DOWNLOAD_DIR ${PROJECT_BINARY_DIR}/tre-down
                    CONFIGURE_COMMAND cd ${CMAKE_CURRENT_SOURCE_DIR}/tre/ && ./utils/autogen.sh && cd ${PROJECT_BINARY_DIR}/tre-build && ${CMAKE_CURRENT_SOURCE_DIR}/tre/configure --prefix=${CMAKE_CURRENT_SOURCE_DIR}/local
)

# pcre2
ExternalProject_Add(libpcre2
                    URL https://ftp.pcre.org/pub/pcre/pcre2-10.23.tar.gz
                    PREFIX ${CMAKE_CURRENT_SOURCE_DIR}
                    SOURCE_DIR pcre2
                    TMP_DIR ${PROJECT_BINARY_DIR}/pcre2-tmp
                    STAMP_DIR ${PROJECT_BINARY_DIR}/pcre2-stamp
                    BINARY_DIR ${PROJECT_BINARY_DIR}/pcre2-build
                    DOWNLOAD_DIR ${PROJECT_BINARY_DIR}/pcre2-down
                    CONFIGURE_COMMAND ${CMAKE_CURRENT_SOURCE_DIR}/pcre2/configure --prefix=${CMAKE_CURRENT_SOURCE_DIR}/local --enable-jit --enable-unicode
)
