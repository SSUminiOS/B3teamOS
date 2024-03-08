all: minios

minios: kernel/kernel.c
	gcc -o minios kernel/kernel.c -lreadline

clean: 
	rm -f minios
