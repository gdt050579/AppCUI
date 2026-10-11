function (create_example example_name)
	set(PROJECT_NAME ${example_name})
	
	include_directories(../../AppCUI/include)
	
	add_executable(${PROJECT_NAME} main.cpp)
	add_dependencies(${PROJECT_NAME} AppCUI)
	
	target_link_libraries(${PROJECT_NAME} PRIVATE AppCUI)
	set_target_properties(${PROJECT_NAME} PROPERTIES FOLDER "Examples")

	if (EMSCRIPTEN)
		# <name>.mjs + <name>.wasm + <name>.html, next to the shared host script and font
		set_target_properties(${PROJECT_NAME} PROPERTIES SUFFIX ".mjs")
		if (CMAKE_RUNTIME_OUTPUT_DIRECTORY)
			set(web_output_dir "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}")
		else()
			set(web_output_dir "${CMAKE_CURRENT_BINARY_DIR}")
		endif()
		get_filename_component(web_dir "${CMAKE_CURRENT_FUNCTION_LIST_DIR}/../AppCUI/web" ABSOLUTE)
		get_filename_component(font_file "${CMAKE_CURRENT_FUNCTION_LIST_DIR}/../AppCUI/resources/JuliaMono-Regular.ttf" ABSOLUTE)
		set(APPCUI_WEB_APP ${PROJECT_NAME})
		configure_file("${web_dir}/index.html.in" "${web_output_dir}/${PROJECT_NAME}.html" @ONLY)
		configure_file("${web_dir}/appcui-host.js" "${web_output_dir}/appcui-host.js" COPYONLY)
		configure_file("${web_dir}/serve.py" "${web_output_dir}/serve.py" COPYONLY)
		configure_file("${font_file}" "${web_output_dir}/JuliaMono-Regular.ttf" COPYONLY)
	endif()
endfunction()

