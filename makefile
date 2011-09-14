all:
	gcc ps_print.c -o ps_print
MACOSX:
	gcc -D MACOSX ps_print.c -o ps_print-MACOSX