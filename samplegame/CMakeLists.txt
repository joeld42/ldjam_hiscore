include_directories(
	${CMAKE_SOURCE_DIR}/
	/Users/joeld/Toolkits/tk_raylib/src/
	#/usr/local/include
)
link_directories(
	/Users/joeld/Toolkits/tk_raylib/release/osx/
	/usr/local/Cellar/glfw3/3.2.1/lib 	
)

set ( LIBS ${LIBS}
		"-framework OpenGL"
		"-framework OpenAl"
		"-framework Cocoa"
		#"-framework AudioToolbox"
		raylib
		glfw3
      )

add_executable(bricks ldjam_bricks.cpp)
target_compile_options( bricks PUBLIC -fno-rtti)
target_link_libraries( bricks ${LIBS} )
set_target_properties( bricks PROPERTIES
		       		 RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/ )
