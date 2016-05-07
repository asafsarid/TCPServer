TCP Server
----------

This is a TCP server class that accepts incoming connections and handles incoming data using sockets.
The TCPServer class uses SocketWrapper class for all the operations on the sockets.
Main function is currently located inside TCPServer.cpp for testing purpose.

###Usage:

TCPServer constructor receives 2 arguments:
1. Port as integer
2. Callback function
Note: The callback function should receive a string as an argument and return void.

Start the server by calling `start()` method.

Terminate the server by calling `stop()` method.

**Compile:**
```
make all
```

**Run:**
```
./bin/TCPServer
```
