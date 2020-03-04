#
# https://wiki.qt.io/Qt_for_Python/Considerations
# https://doc.qt.io/qtforpython/shiboken2
#

cmake_minimum_required(VERSION 3.12)

project(python)

# FindREDasm.config
list(APPEND CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/LibREDasm/redasm")

find_package(REDasm REQUIRED)
find_package(Shiboken2 2.0.0 REQUIRED)
find_package(Python3 COMPONENTS Development REQUIRED)

get_target_property(SHIBOKEN_EXECUTABLE Shiboken2::shiboken2 IMPORTED_LOCATION_RELEASE)

set(GENERATED_FOLDER ${CMAKE_BINARY_DIR}/generated)
set(TYPESYSTEM_FILE ${CMAKE_SOURCE_DIR}/generator/typesystem/redasm.xml)
set(BINDING_HEADER ${CMAKE_BINARY_DIR}/bindings.h)
set(REDASM_BASE ${CMAKE_SOURCE_DIR}/LibREDasm)
set(REDASM_API_BASE ${REDASM_BASE}/redasm)

message("Generating Root Header")
execute_process(COMMAND ${SHIBOKEN_PYTHON_INTERPRETER} ${CMAKE_SOURCE_DIR}/generator/generate.py ${BINDING_HEADER} ${REDASM_BASE})

message("Generating Bindings")
file(REMOVE_RECURSE ${GENERATED_FOLDER}) # Clear generated files first

execute_process(COMMAND ${SHIBOKEN_EXECUTABLE} -I${REDASM_BASE}:${REDASM_API_BASE}/libs --output-directory=${GENERATED_FOLDER} ${BINDING_HEADER} ${TYPESYSTEM_FILE})

file(GLOB_RECURSE PLUGIN_HEADERS plugin/*.h*)
file(GLOB_RECURSE PLUGIN_SOURCES plugin/*.cpp)
file(GLOB_RECURSE GENERATED_HEADERS ${GENERATED_FOLDER}/*.h)
file(GLOB_RECURSE GENERATED_SOURCES ${GENERATED_FOLDER}/*.cpp)

add_subdirectory(LibREDasm)

redasm_plugin(${PROJECT_NAME} ${GENERATED_HEADERS} ${GENERATED_SOURCES} ${PLUGIN_HEADERS} ${PLUGIN_SOURCES})
target_link_libraries(${PROJECT_NAME} PRIVATE ${Python3_LIBRARIES} Shiboken2::libshiboken)
target_include_directories(${PROJECT_NAME} PRIVATE ${Python3_INCLUDE_DIRS})

add_dependencies(Shiboken2::shiboken2 LibREDasm)
add_dependencies(${PROJECT_NAME} Shiboken2::shiboken2)