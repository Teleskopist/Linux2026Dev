GENERATES = prog prog-a prog-so README liboutput_static.a liboutput.so
TEST_OUTPUTS = $(foreach binary,prog prog-a prog-so,$(foreach count,0 1 3,test-$(binary)-$(count).out))
TRASH = *.o *~ o.* $(TEST_OUTPUTS)
CFLAGS = -Wall

all:    README prog prog-a prog-so liboutput_static.a liboutput.so

.SECONDARY:	fun.o const.o

fun.o const.o: CFLAGS += -fPIC

liboutput_static.a: fun.o const.o
	$(AR) rcs $@ $^

liboutput.so: fun.o const.o
	$(CC) $(LDFLAGS) -shared -o $@ $^ $(LDLIBS)

prog: prog.o fun.o const.o
	$(CC) $(LDFLAGS) -o $@ $^ $(LDLIBS)

prog-a: prog.o liboutput_static.a
	$(CC) $(LDFLAGS) -o $@ $^ $(LDLIBS)

prog-so: prog.o liboutput.so
	$(CC) $(LDFLAGS) -o $@ prog.o -L. -loutput -Wl,-rpath,'$$ORIGIN' $(LDLIBS)

.PHONY: test clean distclean

test: prog prog-a prog-so
	@status=0; \
	for count in 0 1 3; do \
		case $$count in \
			0) set -- ;; \
			1) set -- M4_Sherman ;; \
			3) set -- cake is_a lie ;; \
		esac; \
		for binary in prog prog-a prog-so; do \
			./$$binary "$$@" > "test-$$binary-$$count.out" 2>&1 || status=1; \
		done; \
		for binary in prog-a prog-so; do \
			if ! cmp "test-prog-$$count.out" "test-$$binary-$$count.out"; then \
				echo "Output mismatch: prog vs $$binary ($$count arguments)" >&2; \
				status=1; \
			fi; \
		done; \
	done; \
	exit $$status

README: prog
	./$< 2> $@

fun.o prog.o:  outlib.h

clean_trash:
	rm -f $(TRASH)

clean:      clean_trash
	rm -rf $(GENERATES)
