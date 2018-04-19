include unix_def.mk

CFLAGS+=-Isrc/include

# GNU make does not support .MAIN
all: main

include src/commands/build.mk
include src/events/build.mk
include src/build.mk

main: $(TGTS)

.c.o:
	$(CC) $(CFLAGS) -c -o $@ $<

# install target
include install.mk

clean:
	$(RM) $(OBJS)
	$(RM) $(TGTS)
