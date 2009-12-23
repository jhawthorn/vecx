
CFLAGS := -O3 $(shell sdl-config --cflags)
LIBS := $(shell sdl-config --libs) -lSDL_gfx
OBJECTS := e6809.o osint.o vecx.o
TARGET := vecx
CLEANFILES := $(TARGET) $(OBJECTS)

$(TARGET): $(OBJECTS)
	gcc $(CFLAGS) -o $@ $^ $(LIBS)

clean:
	$(RM) $(CLEANFILES)

