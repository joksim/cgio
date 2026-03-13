
# List all .cpp source files in this directory. Will not recursively go into subdirectories
FILE(GLOB_RECURSE sources LIST_DIRECTORIES false RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} "*.cpp" )

# Iterate over all of the sources
FOREACH (SOURCE  ${sources})
    MESSAGE("PROCESSING THE FOLLOWING SOURCE FILES")
    MESSAGE(${SOURCE})

    CMAKE_PATH(GET SOURCE PARENT_PATH TMP)
    STRING(REPLACE "/" "-" target_name ${TMP})
    MESSAGE(${target_name})
    # Extract just the filename with extensions
    # cmake_path(GET SOURCE FILENAME file_name)
    # message(STATUS "Processing source " ${file_name})

    # Extract just the filename without extension for creating the executable name
    # cmake_path(GET SOURCE STEM target_name)
    # message(STATUS "Creating executable for: " ${target_name})

    add_executable(${target_name} ${SOURCE})
    target_link_libraries(${target_name} ${LIBS})
ENDFOREACH()