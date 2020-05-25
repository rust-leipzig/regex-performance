# check rust package manager
find_program(RUST_C rustc)

if(NOT RUST_C)
    message(FATAL_ERROR "Rust compiler 'rustc' not found.")
else()
    execute_process(COMMAND ${RUST_C} --version OUTPUT_VARIABLE RUST_C_VERSION)
    string(STRIP ${RUST_C_VERSION} RUST_C_VERSION)
    message("-- Found rust compiler: ${RUST_C_VERSION}")
endif()

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/.cargo/config.in ${CMAKE_CURRENT_SOURCE_DIR}/.cargo/config)

if(CMAKE_BUILD_TYPE MATCHES "^Release")
    list(APPEND CARGO_BUILD_OPT "--release")
endif()

add_custom_target(librregex ALL
    # create rust regex library
    COMMAND cargo build -v ${CARGO_BUILD_OPT}
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
)
