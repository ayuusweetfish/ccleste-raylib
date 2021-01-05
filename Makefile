LD = $(CC)

CFLAGS += -Iraylib/src

# macOS
LDEXTRAFLAGS = raylib/src/libraylib.a -framework OpenGL -framework OpenAL -framework IOKit -framework CoreVideo -framework Cocoa

main: main.o raylib.target
	$(LD) -o $@ $< $(LDEXTRAFLAGS)

raylib.target:
	$(MAKE) -C raylib/src

.PHONY: raylib.target
