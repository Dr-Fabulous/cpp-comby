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
FetchContent_Declare(cpp_result
		     GIT_REPOSITORY "https://github.com/Dr-Fabulous/cpp-result.git"
		     GIT_TAG main)

FetchContent_MakeAvailable(cpp_result)

if(NOT cpp_result_POPULATED)
	message(FATAL_ERROR "failed to populate cpp-result")
endif()

add_subdirectory(src)

include(CTest)
add_subdirectory(test)
