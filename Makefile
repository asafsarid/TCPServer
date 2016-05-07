# All Targets
all: TCPServer

# Tool invocations
# Executable "TCPServer" depends on the files TCPServer.o and SocketWrapper.o
TCPServer: bin/TCPServer.o bin/SocketWrapper.o
	@echo 'Finish compiling classes'	
	@echo 'Building target...'
	g++ -o bin/TCPServer bin/TCPServer.o bin/SocketWrapper.o -lpthread
	@echo 'Finished building target: TCPServer'
	@echo ' '

# Depends on the source and header files
bin/TCPServer.o: src/TCPServer.cpp src/TCPServer.h
	@echo 'Compiling TCPServer Class...'
	g++ -c -o bin/TCPServer.o src/TCPServer.cpp -std=c++11

bin/SocketWrapper.o: src/SocketWrapper.cpp src/SocketWrapper.h
	@echo 'Compiling SocketWrapper Class...'
	g++ -c -o bin/SocketWrapper.o src/SocketWrapper.cpp -std=c++11

#Clean the build directory
clean: 
	rm -f bin/*
