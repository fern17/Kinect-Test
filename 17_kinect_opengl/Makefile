CC = g++
LD = g++
CFLAGS=-g -Wall
LIBS = -lGL -lGLU -lglut

INCLUDE = -I/usr/local/include/libfreenect -I/usr/include/GL
FREE_LIBS = -L/usr/local/lib -lfreenect
 
OBJECTS = main.o
PROG = mycppview

all:$(PROG)

$(PROG): $(OBJECTS)
	$(LD) $(LDFLAGS) $(INCLUDE) $(FREE_LIBS) $(LIBS) $(OBJECTS) -o $(PROG)

%.o: %.cpp
	$(CC) $(CFLAGS)  $(LIBS) -c $<

clean:
	rm -rf *.o $(PROG)

