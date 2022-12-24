all:
	gcc -ansi -Wall -pedantic -lpthread -std=gnu99 -g gol_data.c -o gol_data
	gcc -ansi -Wall -pedantic -lpthread -std=gnu99 -g gol_task.c -o gol_task

gol_data:
	gcc -ansi -Wall -pedantic -lpthread -std=gnu99 -g gol_data.c -o gol_data

gol_task:
	gcc -ansi -Wall -pedantic -lpthread -std=gnu99 -g gol_task.c -o gol_task

clean:
	rm gol_data gol_task