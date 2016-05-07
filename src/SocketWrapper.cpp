/*
 * SocketWrapper.cpp
 *
 *  Created on: 	06 May 2016
 *  Modified on: 	07 May 2016
 *
 *      Author: Asaf Sarid
 */

//General Includes
#include <iostream>
#include <cstring>			//Needed for memset and string functions
//Socket Includes
#include <sys/socket.h>		//Core socket functions and data structures
#include <sys/types.h>		//Core socket functions and data structures
#include <sys/time.h>		//Needed for timeval struct
#include <netdb.h>			//Functions for translating protocol names and host names into numeric addresses
#include <fcntl.h>			//fcntl() function and defines
#include <arpa/inet.h>
#include <unistd.h>			//Needed for I/O system calls
//Class Headers
#include "SocketWrapper.h"

using namespace std;

SocketWrapper::SocketWrapper() {
	//Clearing master and read sets
	FD_ZERO(&master_fds);
	FD_ZERO(&read_fds);
}
SocketWrapper::~SocketWrapper() {
}

int SocketWrapper::socket_bind(int port){
	int listener_fd;						//Listener socket descriptor
	struct addrinfo hints;					//Holds wanted settings for the listener socket
	struct addrinfo *server_info_list;		//A list of possible information to create socket

	//All the other fields in the addrinfo struct (hints) must contain 0
	memset(&hints, 0, sizeof hints);

	//Initialize connection information
	hints.ai_family = AF_UNSPEC;			//Supports IPv4 and IPv6
	hints.ai_socktype = SOCK_STREAM;		//Reliable Stream (TCP)
	hints.ai_flags = AI_PASSIVE;			//Assign local host address to socket

	//Get address information
	int err;
	err = getaddrinfo(NULL, to_string(port).c_str(), &hints, &server_info_list);
	if (err != 0){
		cerr << "getaddrinfo: " << gai_strerror(err) << endl;
		exit(EXIT_STATUS);
	}

	//Go over list and try to create socket and bind
	addrinfo* p;
	for(p = server_info_list;p != NULL; p = p->ai_next) {

		//Create the socket - system call that returns the file descriptor of the socket
		listener_fd = socket(p->ai_family, p->ai_socktype,p->ai_protocol);
        if (listener_fd == -1) {
            continue; //try next
        }

        //Make sure the port is not in use. Allows reuse of local address (and port)
        int yes = 1;
        if (setsockopt(listener_fd,SOL_SOCKET,SO_REUSEADDR,&yes,sizeof(int)) == -1) {
            perror("setsockopt");
            exit(EXIT_STATUS);
        }

        //Bind socket to specific port (p->ai_addr holds the address and port information)
        if (bind(listener_fd, p->ai_addr, p->ai_addrlen) == -1) {
			close(listener_fd);
			continue; //try next
		}

        break; //success
	}

	//No one from the list succeeded - failed to bind
	if (p == NULL)  {
	        cerr << "failed to bind" << endl;
	        exit(EXIT_STATUS);
	}

	//If we got here we successfully created a socket to listen on
	//Free list as we no longer need it
	freeaddrinfo(server_info_list);

	//return the listener socket descriptor
	return listener_fd;
}

void SocketWrapper::start_listen(int listener_fd, int backlog){
	//Listen on the given port for incoming connections (Maximum BACKLOG waiting connections in queue)
	if (listen(listener_fd, backlog) == -1){
		perror("listen");
		exit(EXIT_STATUS);
	}
}

void SocketWrapper::set_nonblock(int socket){
	int flags;
	// save current flags
	flags = fcntl(socket, F_GETFL, 0);
	if (flags == -1)
		perror("fcntl");
	// set socket to be non-blocking
	if (fcntl(socket, F_SETFL, flags | O_NONBLOCK) == -1)
		perror("fcntl");
}

void SocketWrapper::create_sets(int listener_fd){
	//clear master and read sets
    FD_ZERO(&master_fds);
    FD_ZERO(&read_fds);
    // add the listener socket descriptor to the master set
    FD_SET(listener_fd, &master_fds);
}

void SocketWrapper::start_select(int max_fd, int timeout_sec, int timeout_usec){
    //Set timeout values (for waiting on select())
	struct timeval timeout = {timeout_sec, timeout_usec};
	//Copy all available (open) sockets to the read set
	read_fds = master_fds;

	//Select - 	modifies read_fds set to show which sockets are ready for reading
	//			if none are ready, it will timeout after the given timeout values
	int sel = select(max_fd+1, &read_fds, NULL, NULL, &timeout);
	if (sel == -1) {
		perror("select");
		exit(EXIT_STATUS);
	}
}

int SocketWrapper::check_new_connection(int listener_fd, int max_fd){
	//Check if listener socket is in read set (has changed and has an incoming connection to accept)
	if (FD_ISSET(listener_fd,&read_fds)){
		int client_fd;
		struct sockaddr_storage their_addr;
		socklen_t addr_size = sizeof their_addr;

		//Accept the incoming connection, save the socket descriptor (client_fd)
		client_fd = accept(listener_fd, (struct sockaddr *)&their_addr, &addr_size);
		if (client_fd == -1){
			perror("accept");
		}
		else{ //If connection accepted
			//Set this socket to be non-blocking
			set_nonblock(client_fd);
			//Add socket to the master set
			FD_SET(client_fd, &master_fds);
			//Update max_fd
			if (client_fd > max_fd)
				max_fd = client_fd;
			//Print incoming connection
			if (their_addr.ss_family == AF_INET){
				//IPv4
				char ip_as_string[INET_ADDRSTRLEN];
				inet_ntop(their_addr.ss_family,&((struct sockaddr_in *)&their_addr)->sin_addr,ip_as_string, INET_ADDRSTRLEN);
				cout << "New connection from " << ip_as_string << " on socket " << client_fd << endl;
			} else if(their_addr.ss_family == AF_INET6){
				//IPv6
				char ip_as_string[INET6_ADDRSTRLEN];
				inet_ntop(their_addr.ss_family,&((struct sockaddr_in6 *)&their_addr)->sin6_addr,ip_as_string, INET6_ADDRSTRLEN);
				cout << "New connection from " << ip_as_string << " on socket " << client_fd << endl;
			}
		}
	}
	return max_fd;
}

int SocketWrapper::receive_data(int client_fd, int buffer_size, string& data){
	//set buffer with given buffer_size
	char buf[buffer_size];

	//Check if socket is in read set (has data or has closed the connection)
	if (FD_ISSET(client_fd, &read_fds)){
		int bytes;

		//Receive data
		bytes = recv(client_fd, buf, sizeof buf, 0);

		//Connection has been closed by client
		if (bytes <= 0){
			if (bytes == -1)
				perror("recv");
			//close socket
			close(client_fd);
			//remove from master set
			FD_CLR(client_fd, &master_fds);
		}
		//Some data received - bytes > 0
		else{
			//Add NUL terminator
			buf[bytes] = '\0';
			if (buf[bytes-1] == '\n')
			    buf[bytes-1] = '\0';
			//Assign received data to the given string
			data.assign(buf);
		}
		return bytes;
	}
	//socket is not in read set
	return -1;
}

void SocketWrapper::respond_ok(int client_fd){
	//Send "OK" to given client
	char respond[4] = "OK\n";
    if (send(client_fd, respond, sizeof respond, 0) == -1) {
        perror("send");
    }
}

void SocketWrapper::close_all(int max_fd){
	//Close all socket descriptors, this will terminate all connections
    for (int i=0; i < max_fd+1; i++){
    	//If socket is in the master set it means it is still open - so close it
    	if (FD_ISSET(i, &master_fds))
    		close(i);
    }
}
