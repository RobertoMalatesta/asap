
list(APPEND MAIN_APP_SRC
        #FONTS
        src/ui/fonts/fonts.h
        src/ui/fonts/Roboto-Black.cpp
        src/ui/fonts/Roboto-BlackItalic.cpp
        src/ui/fonts/Roboto-Bold.cpp
        src/ui/fonts/Roboto-BoldItalic.cpp
        src/ui/fonts/Roboto-Italic.cpp
        src/ui/fonts/Roboto-Light.cpp
        src/ui/fonts/Roboto-LightItalic.cpp
        src/ui/fonts/Roboto-Medium.cpp
        src/ui/fonts/Roboto-MediumItalic.cpp
        src/ui/fonts/Roboto-Regular.cpp
        src/ui/fonts/Roboto-Thin.cpp
        src/ui/fonts/Roboto-ThinItalic.cpp
        src/ui/fonts/Inconsolata-Regular.cpp
        src/ui/fonts/Inconsolata-Bold.cpp
        src/ui/fonts/material_design_icons.cpp
        src/ui/fonts/material_design_icons.h
        #
        src/ui/debug_ui.h
        src/ui/style/theme.h
        src/ui/style/theme.cpp
        src/ui/log/sink.h
        src/ui/log/sink.cpp
	#
        src/KHR/khrplatform.h
        src/glad/gl.h
        src/imgui_impl_opengl3.h
        src/imgui_impl_opengl3.cpp
        src/imgui_impl_glfw.h
        src/imgui_impl_glfw.cpp
        src/glad.c
		src/runner.h
		src/console_runner.h
		src/console_runner.cpp
		src/imgui_runner.h
		src/imgui_runner.cpp
		src/main.cpp
        )

# Needed for spdlog
find_package(Threads REQUIRED)

set(MAIN_APP_LIBRARIES
	    asap::common
        spdlog
		date_interface
        Boost::program_options
        Boost::thread
        imgui
        glfw
        Threads::Threads
        )


list(APPEND MAIN_APP_INCLUDE_DIRS
        ${CMAKE_CURRENT_SOURCE_DIR}/src
        )

asap_executable(TARGET
        TARGET
        asap_app
        SOURCES
        ${MAIN_APP_SRC}
        INCLUDE_DIRS
        ${MAIN_APP_INCLUDE_DIRS}
        LIBRARIES
        ${MAIN_APP_LIBRARIES}
)
set_tidy_target_properties(ndagent)

set_cppcheck_command()
