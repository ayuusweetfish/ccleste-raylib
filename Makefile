LD = $(CC)

CFLAGS += -Iraylib/src -g

# macOS
LDEXTRAFLAGS = raylib/src/libraylib.a -framework OpenGL -framework OpenAL -framework IOKit -framework CoreVideo -framework Cocoa

OBJS = main.o p8.o celeste.o

main: $(OBJS) raylib.target
	$(LD) -o $@ $(OBJS) $(LDEXTRAFLAGS)

celeste.o: ccleste/celeste.c
	$(CC) -c $^ -o $@ $(CFLAGS) $(CPPFLAGS)

raylib.target:
	$(MAKE) -C raylib/src

.PHONY: raylib.target
