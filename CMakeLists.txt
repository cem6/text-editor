cmake_minimum_required(VERSION 3.27)
project(editor)

set(CMAKE_CXX_STANDARD 17)

add_executable(editor
        src/main.cpp
        src/settings.cpp
        src/settings.h
        src/window.h
        src/filehandling.h
        src/cursor.h
)

target_link_libraries(editor
        sfml-graphics 
        sfml-window 
        sfml-system
)

set_target_properties(editor
	PROPERTIES
		OUTPUT_NAME "out"
)
