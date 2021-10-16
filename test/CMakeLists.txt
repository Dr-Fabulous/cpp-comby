if(NOT BUILD_TESTING)
	return()
endif()

file(GLOB ALL_SRC "*.cpp")
set(TESTS "")

foreach(PATH ${ALL_SRC})
	string(REGEX REPLACE ".*[\\/](.*)\.cpp" "\\1" PATH_NAME "${PATH}")
	set(NAME "${PROJECT_NAME}-test-${PATH_NAME}")

	add_executable(${NAME} ${PATH})
	target_compile_features(${NAME} PUBLIC cxx_std_20)
	target_include_directories(${NAME} PUBLIC inc)
	target_link_libraries(${NAME} PUBLIC ${PROJECT_NAME})

	add_test(NAME "${PATH_NAME}"
		 COMMAND ${CMAKE_CURRENT_BINARY_DIR}/${NAME})

	list(APPEND TESTS "${NAME}")
endforeach()

if(IS_MAIN_PROJECT)
	add_custom_target(check
			  COMMAND ${CMAKE_CTEST_COMMAND} --verbose
			  DEPENDS "${TESTS}")
else()
	add_custom_target(cpp-comby-check
			  COMMAND ${CMAKE_CTEST_COMMAND} --verbose
	  		  DEPENDS "${TESTS}")
endif()
