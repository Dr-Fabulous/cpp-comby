cmake_minimum_required(VERSION 3.18.0)

find_package(Git)

if(NOT Git_FOUND)
	message(FATAL_ERROR "could not find Git")
endif()

execute_process(COMMAND git describe --tags --abbrev=0
		RESULT_VARIABLE  _RET
		OUTPUT_VARIABLE  _TAG
		OUTPUT_STRIP_TRAILING_WHITESPACE)

if(_RET)
	message(FATAL_ERROR "failed to get git tag")
endif()

project(cpp-comby
	LANGUAGES CXX
	VERSION "${_TAG}")

include(FetchContent)
FetchContent_Declare(cpp_tag_invoke
		     GIT_REPOSITORY "https://github.com/Dr-Fabulous/cpp-tag-invoke.git"
		     GIT_TAG main)

FetchContent_MakeAvailable(cpp_tag_invoke)

if(NOT cpp_tag_invoke_POPULATED)
	message(FATAL_ERROR "failed to populate cpp-tag-invoke")
endif()

if("^${CMAKE_SOURCE_DIR}$" STREQUAL "^${PROJECT_SOURCE_DIR}$")
	set(IS_MAIN_PROJECT ON)
	message(STATUS "adding ${PROJECT_NAME} v${_TAG}")
else()
	set(IS_MAIN_PROJECT OFF)
	message(STATUS "adding ${PROJECT_NAME} v${_TAG} as a subproject")
endif()

add_subdirectory(comby)

include(CTest)
add_subdirectory(test EXCLUDE_FROM_ALL)
