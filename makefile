src = $(wildcard *.c)

my_chip-8.out: $(src) $(wildcard *.h)
	cc -o my_chip-8.out $(src) -lncurses -g