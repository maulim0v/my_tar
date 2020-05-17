output: main.o my_tar_main.o
	gcc main.o my_tar_main.o -g3 -fsanitize=address -o my_tar

main.o: main.c
	gcc -c main.c

tar_main.o: my_tar_main.c my_tar_main.h
	gcc -c my_tar_main.c

clean:
	rm *.o my_tar