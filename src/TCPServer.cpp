/*
 * TCPServer.cpp
 *
 *  Created on: 	29 Apr 2016
 *  Modified on: 	07 May 2016
 *
 *      Author: Asaf Sarid
 */

//General Includes
#include <iostream>
#include <thread>			//Needed for std::thread
#include <cstring>			//Needed for memset and string functions
#include <queue>			//Needed for std::queue
#include <unistd.h>			//Needed for sleep
//Class Headers
#include "TCPServer.h"
#include "SocketWrapper.h"

/**
 * Constructor.
 * Initialize _running flag as true and allocates data queue on heap
 */
TCPServer::TCPServer(int port, callback_t callback) : _port(port),_callback(callback),
														_running(true), _data(new queue<string>), _thread() {
}

/**
 * Copy constructor.
 * Copies only port and callback function. Data queue is not copied!
 */
TCPServer::TCPServer(const TCPServer &other):_port(other.get_port()),_callback(other.get_callback()),
												_running(true), _data(new queue<string>), _thread() {
}

/**
 * Assignment Operator.
 * Assigns only port and callback function. Data queue is not copied, running flag is set to true.
 */
TCPServer& TCPServer::operator=(const TCPServer &other){
	if (&other == this)
		return *this;
	else {
		this->_port = other.get_port();
		this->_callback = other.get_callback();
		this->_running = true;
		return *this;
	}
}

/**
 * Destructor.
 * Sets flag to false and if thread is joinable (meaning that no one called stop() before), waits for the thread to finish.
 * Deallocates data queue.
 */
TCPServer::~TCPServer() {
	_running = false;
	if (_thread.joinable())
		_thread.join();
	delete _data;
}

/**
 * Setting flag to true and starting the servers thread with the main run() routine.
 */
void TCPServer::start(){
	_running = true;
	_thread = thread(&TCPServer::run, this);
}

/**
 * Stops the server by setting _running flag to false.
 * Waiting for the servers thread to finish its execution.
 */
void TCPServer::stop(){
	_running = false;
	if (_thread.joinable())
		_thread.join();
}

/**
 * This method is the servers main routine.
 * 		1. Creates a listener socket and bind it to the given port
 * 		2. Start listening on the given port.
 * 		3. Sets all sockets to be non-blocking.
 * 		4. Creates a set to hold all active sockets (connection)
 * 		5. While running flag is true:
 * 			5.1 Check for changes in active sockets.
 * 			5.2 If listener socket has new connection - Accept it and add to set.
 * 			5.3 If some client socket received data (bytes > 0) - Handle it.
 * 		6. Close all sockets - terminate server.
 */
void TCPServer::run(){

	int listener_fd;		//Listener socket
	int max_fd;				//Hold the maximum file descriptor number from the active sockets

	//Wraps the sockets detailed implementation. See SocketWrapper for details.
	SocketWrapper socket_wrapper;

	// 1.
	listener_fd = socket_wrapper.socket_bind(_port);

	// 2.
	socket_wrapper.start_listen(listener_fd, BACKLOG);
	cout << "Listening on port " << _port << "..." << endl;

    // 3.
    socket_wrapper.set_nonblock(listener_fd);

    // 4.
    socket_wrapper.create_sets(listener_fd);
    max_fd = listener_fd;

    // 5.
    while(_running){

    	//5.1
    	socket_wrapper.start_select(max_fd, TIMEOUT_SEC, TIMEOUT_USEC);
		for (int i=0; i < max_fd+1; i++){

			// 5.2
			if (i == listener_fd){
				max_fd = socket_wrapper.check_new_connection(listener_fd, max_fd);
			}

			//5.3
			else {
				string data;
				int bytes = socket_wrapper.receive_data(i, BUFFER_SIZE, data);
				if (bytes > 0) {
					socket_wrapper.respond_ok(i);
					handler(data);
				}
			}
		}
    }

    // 6.
    socket_wrapper.close_all(max_fd);
}

/**
 * Handles incoming data - adding data to queue and calling callback function
 */
void TCPServer::handler(string incoming_data){

	//Call the callback function with the incoming data
	_callback(incoming_data);

	//Add data to the internal data queue
	_data->push(incoming_data);
}

/**
 * Pop data from queue if available.
 */
string TCPServer::pop_data(){
	if (!_data->empty()){
		string data = _data->front();
		cout << "Pop!!! " << _data->front() << endl;
		_data->pop();
		return data;
	}
	else{
		cout << "No data in queue" << endl;
		return "";
	}
}

/**
 * Get port
 */
int TCPServer::get_port() const{
	return _port;
}

/**
 * Get callback function
 */
callback_t TCPServer::get_callback() const{
	return _callback;
}

//=============== End of TCPServer class implementation ===============//



//======================================================================//
//=============== Code below is for testing purpose only ===============//
//======================================================================//
void print(string print){
	cout << "Callback Function: " << print << endl;
}
void print2(string print){
	cout << "Callback Function2: " << print << endl;
}

int main (int argc, char *argv[]) {
	TCPServer tcp_server(5555, &print);
	TCPServer tcp_server2(3333, &print2);

	tcp_server.start();
	tcp_server2.start();
	sleep(60);
	tcp_server.pop_data();
	tcp_server.pop_data();
	tcp_server2.stop();
	cout << "server on 3333 stopped" << endl;
	tcp_server2.pop_data();
	tcp_server2.pop_data();
	sleep(30);
}

