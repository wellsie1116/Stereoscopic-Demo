CC=gcc
CFLAGS=`pkg-config --cflags glib-2.0`
LDFLAGS=-lGL -lglut -lGLU -lGLEW `pkg-config --libs glib-2.0`

targets: tex.o vect.o shader.o targets.o

clean:
	rm *.o targets
