default_target: all

all:
	gcc utils.c rx.c -o rx
	gcc utils.c tx.c -o tx
