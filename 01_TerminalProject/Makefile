Show: Show.c
	gcc -Wall -g -O2 -Wextra -std=c11 Show.c -lncursesw -o Show

install: Show
	install -D Show $(DESTDIR)/usr/bin/Show

clean:
	rm -f Show
