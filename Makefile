
LDFLAGS = -lmilter -L/usr/lib/libmilter/ -lcurl
INCFLAGS = 
CFLAGS += -pipe -Wall -pedantic -O2 -fstack-protector-all
DEBUGCFLAGS = -pipe -Wall -pedantic -Werror -ggdb -Wno-error=unused-variable -fstack-protector-all

objs=\
bin/main.o\


all: $(objs)
	$(CC) $(CFLAGS) $(LDFLAGS) $(objs) -o bin/milter-notify

%.o: src/%.c
	$(CC) -std=gnu11 $(CFLAGS) $(INCFLAGS) $< -c -o bin/$@

debug:
	$(CC) -std=gnu11 $(DEBUGCFLAGS) $(INCFLAGS) $(LDFLAGS) src/*.c -o milter-notify-debug

clean:
	rm -f milter-notify milter-notify-debug $(objs)

run: all
	./milter-notify -p inet:6969@localhost

