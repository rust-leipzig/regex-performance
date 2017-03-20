# check rust package manager
find_program(RUST_CARGO cargo)

if(NOT RUST_CARGO)
    message(FATAL_ERROR "Rust package manager 'cargo' not found.")
else()
    execute_process(COMMAND ${RUST_CARGO} --version OUTPUT_VARIABLE CARGO_VERSION)
    string(STRIP ${CARGO_VERSION} CARGO_VERSION)
    message("-- Found rust packet manager: ${CARGO_VERSION}")
endif()

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/.cargo/config.in ${CMAKE_CURRENT_SOURCE_DIR}/.cargo/config)

if(CMAKE_BUILD_TYPE MATCHES "^Release")
    set(RUST_CARGO_RELEASE_OPT "--release")
endif()

add_custom_target(librregex ALL
    # create rust regex library
    COMMAND ${RUST_CARGO} build ${RUST_CARGO_RELEASE_OPT}
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
)
