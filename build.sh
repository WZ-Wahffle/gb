#!/bin/sh

function build() {
    mkdir -p out
    g++ -c -o src/ui.o src/ui.cpp -Isrc/include/ -Wall -Wextra -Werror -Wno-unused-function
	gcc -g -o out/gbc src/*.c src/carts/*.c src/ui.o -Isrc/include/ -Lsrc/lib/ -lraylib -lSDL2 -lm -lrlImGui -limgui -Wall -Wextra -Werror -lstdc++ -Wno-unused-function
}

function build_raylib() {
    mkdir -p src/include
	mkdir -p src/lib
	sed -i 's/$$(RAYLIB_SRC_PATH)\/external\/SDL2\/include/$$(shell pkg-config --cflags sdl2)/g' raylib/src/Makefile
	sed -i 's/$$(RAYLIB_SRC_PATH)\/external\/SDL2\/lib/$$(shell pkg-config --libs sdl2)/g' raylib/src/Makefile
	sed -i 's/-I$$(SDL_INCLUDE_PATH)/$$(SDL_INCLUDE_PATH)/g' raylib/src/Makefile
	sed -i 's/-I$$(SDL_LIBRARY_PATH)/$$(SDL_LIBRARY_PATH)/g' raylib/src/Makefile
	make -s PLATFORM=PLATFORM_DESKTOP_SDL -C raylib/src/
	cp raylib/src/libraylib.a src/lib/libraylib.a
	cp raylib/src/raylib.h src/include/raylib.h
}

function build_rlimgui() {
    mkdir -p src/include
	mkdir -p src/lib
	cp rlImGui/imgui_impl_raylib.h src/include/imgui_impl_raylib.h
	cp rlImGui/rlImGui.h src/include/rlImGui.h
	cd rlImGui
    g++ -c -o rlImGui.o rlImGui.cpp
	ar rcs librlImGui.a rlImGui.o
	cd ..
	cp rlImGui/librlImGui.a src/lib/librlImGui.a
}

function build_imgui() {
    mkdir -p src/include
	mkdir -p src/lib
	cd imgui
	g++ -c *.cpp -DNO_FONT_AWESOME
	ar rcs libimgui.a *.o
	cd ..
	cp imgui/libimgui.a src/lib/libimgui.a
	cp imgui/libimgui.a rlImGui/libimgui.a
	cp imgui/imgui.h src/include/imgui.h
	cp imgui/imgui.h rlImGui/imgui.h
	cp imgui/imconfig.h src/include/imconfig.h
	cp imgui/imconfig.h rlImGui/imconfig.h
}

if [ "$1" = "raylib" ] ; then
    build_raylib
elif [ "$1" = "rlimgui" ] ; then
    build_rlimgui
elif [ "$1" = "imgui" ] ; then
    build_imgui
else
    if [ ! -f src/include/raylib.h ]; then
        echo "building raylib"
        build_raylib
    fi
    if [ ! -f src/include/imgui.h ]; then
        echo "building imgui"
        build_imgui
    fi
    if [ ! -f src/include/rlImGui.h ]; then
        echo "building rlimgui"
        build_rlimgui
    fi
    echo "building"
    build
    echo "finished"
fi
