CPPFLAGS = -std=c++11 -g -Wall -Wno-deprecated -Werror -O2

OS = $(shell uname -s)

ifeq ($(OS), Darwin)
     LIBRARIES = -framework GLUT -framework OpenGL -framework Cocoa -framework IOKit -framework CoreFoundation -framework CoreVideo -lGLFW3 -lglew
else
     LIBRARIES = -lGL -lGLU -lglut -lGLFW3
endif

all: main

opengl_template.o: opengl_template.cpp
	g++ -c opengl_template.cpp -o opengl_template.o $(CPPFLAGS)

main: opengl_template.o
	g++ opengl_template.o -o main $(LIBRARIES)

clean:
	rm -f *~
	rm -f *.o
	rm -f main
