all: tune.c
	gcc -o tune tune.c
clean:
	rm tune
