.SILENT:*

raylib: raylib/src/Makefile
	mkdir -p src/include
	mkdir -p src/lib
	sed -i 's/$$(RAYLIB_SRC_PATH)\/external\/SDL2\/include/$$(shell pkg-config --cflags sdl2)/g' raylib/src/Makefile
	sed -i 's/$$(RAYLIB_SRC_PATH)\/external\/SDL2\/lib/$$(shell pkg-config --libs sdl2)/g' raylib/src/Makefile
	sed -i 's/-I$$(SDL_INCLUDE_PATH)/$$(SDL_INCLUDE_PATH)/g' raylib/src/Makefile
	sed -i 's/-I$$(SDL_LIBRARY_PATH)/$$(SDL_LIBRARY_PATH)/g' raylib/src/Makefile
	make PLATFORM=PLATFORM_DESKTOP_SDL -C raylib/src/
	cp raylib/src/libraylib.a src/lib/libraylib.a
	cp raylib/src/raylib.h src/include/raylib.h

rlImGui: rlImGui/rlImGui.cpp imgui
	mkdir -p src/include
	mkdir -p src/lib
	cp rlImGui/imgui_impl_raylib.h src/include/imgui_impl_raylib.h
	cp rlImGui/rlImGui.h src/include/rlImGui.h
	cd rlImGui && g++ -c -o rlImGui.o rlImGui.cpp
	cd rlImGui && ar rcs librlImGui.a rlImGui.o
	cp rlImGui/librlImGui.a src/lib/librlImGui.a

imgui: imgui/imgui.cpp
	mkdir -p src/include
	mkdir -p src/lib
	cd imgui && g++ -c *.cpp -DNO_FONT_AWESOME
	cd imgui && ar rcs libimgui.a *.o
	cp imgui/libimgui.a src/lib/libimgui.a
	cp imgui/libimgui.a rlImGui/libimgui.a
	cp imgui/imgui.h src/include/imgui.h
	cp imgui/imgui.h rlImGui/imgui.h
	cp imgui/imconfig.h src/include/imconfig.h
	cp imgui/imconfig.h rlImGui/imconfig.h


build: imgui rlImGui raylib
	g++ -c -o src/ui.o src/ui.cpp -Isrc/include/ -Wall -Wextra -Werror -Wno-unused-function
	gcc -g -o out/gb src/*.c src/carts/*.c src/ui.o -Isrc/include/ -Lsrc/lib/ -lraylib -lSDL2 -lm -lrlImGui -limgui -Wall -Wextra -Werror -lstdc++ -Wno-unused-function -DSKIP_BOOT
