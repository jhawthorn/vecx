
CFLAGS := -O3 -Wall -Wextra $(shell sdl2-config --cflags)
LIBS := $(shell sdl2-config --libs) -lSDL2_gfx
OBJECTS := e6809.o e8910.o osint.o vecx.o
TARGET := vecx
CLEANFILES := $(TARGET) $(OBJECTS)

$(TARGET): $(OBJECTS)
	$(CC) $(CFLAGS) -o $@ $^ $(LIBS)

clean:
	$(RM) $(CLEANFILES)

