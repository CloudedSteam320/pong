SRC = .
DST = .
FLAGS = -lraylib -lGL -lm -lpthread -ldl -lrt -lX11

make:
	rm -f main
	g++ $(SRC)/main.cpp $(FLAGS) -o $(DST)/main
	clear
	./$(DST)/main

win:
	x86_64-w64-mingw32-g++ $(SRC)/main.cpp -o pong-win.exe \
    -I /home/arch/Desktop/prog_pcb/low_level/C++/libs/raylib-win/include\
    -L /home/arch/Desktop/prog_pcb/low_level/C++/libs/raylib-win/lib \
    -lraylib -lopengl32 -lgdi32 -lwinmm -mwindows \
	-static