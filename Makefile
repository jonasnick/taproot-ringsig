main: main.c
	gcc -Wextra -Wall -fsanitize=address -fsanitize=undefined -o main main.c
