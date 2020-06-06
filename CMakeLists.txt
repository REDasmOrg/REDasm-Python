#
# https://wiki.qt.io/Qt_for_Python/Considerations
# https://doc.qt.io/qtforpython/shiboken2
#

cmake_minimum_required(VERSION 3.12)

project(RDPython)

# FindREDasm.config
list(APPEND CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/LibREDasm/rdapi")

find_package(REDasm REQUIRED)
find_package(Python3 COMPONENTS Interpreter Development REQUIRED)

add_subdirectory(pybind11)
add_subdirectory(LibREDasm)

set(GENERATED_FOLDER ${CMAKE_BINARY_DIR}/generated)
set(RDAPI_DOC_JSON ${CMAKE_BINARY_DIR}/LibREDasm/docs/doc.json)

#message("Generating Bindings")
file(REMOVE_RECURSE ${GENERATED_FOLDER}) # Clear generated files first
execute_process(COMMAND ${Python3_EXECUTABLE} ${CMAKE_SOURCE_DIR}/generator/generate_bindings.py ${RDAPI_DOC_JSON} ${CMAKE_BINARY_DIR}/generated)
execute_process(COMMAND ${Python3_EXECUTABLE} ${CMAKE_SOURCE_DIR}/generator/generate_header.py ${GENERATED_FOLDER}/rdapi_all.h ${CMAKE_SOURCE_DIR}/LibREDasm)

file(GLOB_RECURSE PLUGIN_HEADERS plugin/*.h*)
file(GLOB_RECURSE PLUGIN_SOURCES plugin/*.cpp)
file(GLOB_RECURSE GENERATED_HEADERS ${GENERATED_FOLDER}/*.h)
file(GLOB_RECURSE GENERATED_SOURCES ${GENERATED_FOLDER}/*.cpp)

redasm_plugin(${PROJECT_NAME} ${GENERATED_HEADERS} ${GENERATED_SOURCES} ${PLUGIN_HEADERS} ${PLUGIN_SOURCES})
set_target_properties(${PROJECT_NAME} PROPERTIES CXX_VISIBILITY_PRESET hidden)
target_link_libraries(${PROJECT_NAME} PRIVATE ${Python3_LIBRARIES} pybind11::embed)

target_include_directories(${PROJECT_NAME} PRIVATE 
	                   ${Python3_INCLUDE_DIRS} 
	                   ${CMAKE_SOURCE_DIR}/LibREDasm
	                   ${CMAKE_SOURCE_DIR}/pybind11/include
			   ${CMAKE_BINARY_DIR}/generated)

add_dependencies(pybind11 LibREDasm)
add_dependencies(${PROJECT_NAME} pybind11)
