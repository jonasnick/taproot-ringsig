main: main.c
	gcc -Wextra -Wall -fsanitize=address -fsanitize=undefined -lsecp256k1 -o main main.c
