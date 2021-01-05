LD = $(CC)

CFLAGS += -Iraylib/src

# macOS
LDEXTRAFLAGS = raylib/src/libraylib.a -framework OpenGL -framework OpenAL -framework IOKit -framework CoreVideo -framework Cocoa

OBJS = main.o p8.o

main: $(OBJS) raylib.target
	$(LD) -o $@ $(OBJS) $(LDEXTRAFLAGS)

raylib.target:
	$(MAKE) -C raylib/src

.PHONY: raylib.target
