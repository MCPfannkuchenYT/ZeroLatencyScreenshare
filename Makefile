default: build

# Sources
C_SOURCES = $(wildcard src/*.c) $(wildcard src/**/*.c)
C_OBJECTS = $(C_SOURCES:.c=.o)

# Skip up-to-date check for targets
.PHONY: default build run clean

%.o: %.c
	gcc -mwindows -c $< -o $@ -lws2_32 -O3

build: $(C_OBJECTS)
	gcc -mwindows -o screenshare.exe $^ -lws2_32 -O3

run: build
	screenshare.exe

clean:
	rm -rf **/*.o *.exe