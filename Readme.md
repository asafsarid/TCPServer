TCP Server
----------

This is a TCP server class that accepts incoming connections and handles incoming data using sockets.
The TCPServer class uses SocketWrapper class for all the operations on the sockets.
Main function is currently located inside TCPServer.cpp for testing purpose.

**Usage:**
TCPServer constructor receives 2 arguments:
	1. Port as integer
	2. Callback function
The callback function should receive a string as an argument and return void.

To start the server use `start()` method.

To terminate the server use `stop()` method.

**Compile:**
```
make all
```

**Run:**
```
./bin/TCPServer
```
