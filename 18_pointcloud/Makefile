all: nubepuntos.bin

CFLAGS=-fPIC -g -Wall -O2 `pkg-config --cflags opencv`
LIBS = `pkg-config --libs opencv` -lGL -lglut -lGLU
INCLUDE = -I/usr/local/include/libfreenect -I/usr/include/GL
FREE_LIBS = -L/usr/local/lib -lfreenect 

nubepuntos.bin:  main.cpp
		$(CXX) $(INCLUDE) $(CFLAGS) $? -o $@  $(LIBS) $(FREE_LIBS)

%.o: %.cpp
		$(CXX) -c $(CFLAGS) $< -o $@

clean:
		rm -rf *.o nubepuntos.bin

