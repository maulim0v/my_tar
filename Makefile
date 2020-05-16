output: main.o tar_main.o
	gcc main.o tar_main.o -g3 -fsanitize=address -o my_tar

main.o: main.c
	gcc -c main.c

tar_main.o: tar_main.c tar_main.h
	gcc -c tar_main.c

clean:
	rm *.o my_tar