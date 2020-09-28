CC = gcc
CCOPTS = -c -g
LINKOPTS = -g

all: shell

shell: shell.o grading_tests.o testrunner.o
	$(CC) $(LINKOPTS) -o $@ $^

shell.o: shell.c grading_tests.h
	$(CC) $(CCOPTS) -o $@ shell.c

testrunner.o: testrunner.c testrunner.h
	$(CC) $(CCOPTS) -o $@ $<

grading_tests.o: grading_tests.c grading_tests.h testrunner.h
	$(CC) $(CCOPTS) -o $@ $<

test: shell
	./shell -test -f0 all

clean:
	rm -rf *.o shell shell.exe *~
