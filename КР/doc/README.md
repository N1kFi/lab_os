./server

./client

strace server:
strace -f -o strace_server.txt ./server

strace client:
strace -f -o strace_client.txt ./client
