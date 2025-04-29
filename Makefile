s:
	gcc server.c -fanalyzer -Wanalyzer-too-complex -o server
	./server

c:
	gcc client.c -fanalyzer -o client
	./client