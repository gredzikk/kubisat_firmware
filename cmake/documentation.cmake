# Find required packages
find_package(Doxygen)
find_program(DOT_EXECUTABLE dot)

# Documentation configuration
if(DOXYGEN_FOUND)
    # Set documentation paths
    set(DOC_OUTPUT_DIR "${CMAKE_CURRENT_SOURCE_DIR}/release/doc")
    set(DOXYGEN_IN "${CMAKE_CURRENT_SOURCE_DIR}/Doxyfile")
    set(DOXYGEN_OUT "${CMAKE_CURRENT_BINARY_DIR}/Doxyfile")

    # Convert input directories list to Doxygen format
    string(REPLACE ";" " " DOXYGEN_INPUT_DIRS 
        "${CMAKE_CURRENT_SOURCE_DIR}/commands \
         ${CMAKE_CURRENT_SOURCE_DIR}/lib \
         ${CMAKE_CURRENT_SOURCE_DIR}/src \
         ${CMAKE_CURRENT_SOURCE_DIR}")

    # Convert exclude patterns to Doxygen format
    string(REPLACE ";" " " DOXYGEN_EXCLUDE_PATTERNS 
        "${CMAKE_CURRENT_SOURCE_DIR}/lib/storage/pico-vfs/* \
         */unity/* \
         */build/* \
         */release/*")

    # Check for Graphviz
    if(DOT_EXECUTABLE)
        set(HAVE_DOT YES)
        get_filename_component(DOT_PATH ${DOT_EXECUTABLE} DIRECTORY)
    else()
        set(HAVE_DOT NO)
        message(STATUS "Graphviz not found - documentation will be generated without graphs")
    endif()

    # Create output directory
    file(MAKE_DIRECTORY ${DOC_OUTPUT_DIR})

    # Configure Doxygen file
    configure_file(${DOXYGEN_IN} ${DOXYGEN_OUT} @ONLY)

    # Add documentation generation to build process
    add_custom_command(
        TARGET main
        PRE_BUILD
        COMMAND ${CMAKE_COMMAND} -E env 
            "HAVE_DOT=${HAVE_DOT}"
            "DOT_PATH=${DOT_PATH}"
            "DOC_OUTPUT_DIR=${DOC_OUTPUT_DIR}"
            "DOXYGEN_INPUT_DIRS=${DOXYGEN_INPUT_DIRS}"
            "DOXYGEN_EXCLUDE_PATTERNS=${DOXYGEN_EXCLUDE_PATTERNS}"
            ${DOXYGEN_EXECUTABLE} ${DOXYGEN_OUT}
        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
        COMMENT "Generating documentation in release/doc"
        VERBATIM
    )
endif()