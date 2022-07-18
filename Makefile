CFLAGS = -g -Wall
LDLIBS = -lncursesw -lmenu -lusb-1.0
INCLUDES = -I./include/
HEADERS = $(wildcard include/*.h)
SOURCES = $(wildcard src/*.c)
OBJECTS = obj/devices/logitech_g300.o obj/devices/drevo_calibur_v2.o obj/helpers.o


all: clean rgbctl


obj/%.o: src/%.c $(HEADERS)
	gcc $(CFLAGS) $(INCLUDES) -c $< -o $@

rgbctl: obj/main.o $(OBJECTS)
	gcc $(CFLAGS) obj/main.o $(OBJECTS) $(LDLIBS) -o rgbctl

clean:
	rm -rf obj/devices/*.o obj/*.o rgbctl
