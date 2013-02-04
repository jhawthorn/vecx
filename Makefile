
CFLAGS := -O3 -Wall -Wextra $(shell sdl-config --cflags)
LIBS := $(shell sdl-config --libs) -lSDL_gfx -lSDL_image
OBJECTS := e6809.o e8910.o osint.o vecx.o
TARGET := vecx
CLEANFILES := $(TARGET) $(OBJECTS)

$(TARGET): $(OBJECTS)
	$(CC) $(CFLAGS) -o $@ $^ $(LIBS)

clean:
	$(RM) $(CLEANFILES)

