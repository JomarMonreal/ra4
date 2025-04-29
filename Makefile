s:
	gcc server.c -o server -lm
	./server $(CPU)

c:
	gcc client.c -o client -lm
	./client

d:
	gcc demo.c -o demo -lm
	./demo
