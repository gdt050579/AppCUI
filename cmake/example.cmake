function (create_example example_name)
	set(PROJECT_NAME ${example_name})
	
	include_directories(../../AppCUI/include)
	
	add_executable(${PROJECT_NAME} main.cpp)
	add_dependencies(${PROJECT_NAME} AppCUI)
	
	target_link_libraries(${PROJECT_NAME} PRIVATE AppCUI)
	set_target_properties(${PROJECT_NAME} PROPERTIES FOLDER "Examples")
endfunction()

