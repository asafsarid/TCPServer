/*
 * TCPServer.h
 *
 *  Created on: 	29 Apr 2016
 *  Modified on: 	07 May 2016
 *
 *      Author: Asaf Sarid
 */
#ifndef TCPSERVER_H_
#define TCPSERVER_H_

#define BACKLOG 5			//Maximum number of connection waiting to be accepted
#define TIMEOUT_SEC 3		//Timeout parameter for select() - in seconds
#define TIMEOUT_USEC 0		//Timeout parameter for select() - in micro seconds
#define BUFFER_SIZE 512		//Incoming data buffer size (maximum bytes per incoming data)

using namespace std;

//Callback type - receives string as argument and returns void
typedef void (*callback_t) (string);

/**
 * TCPServer Class Declaration
 * This class accepts incoming TCP connection on a given port and handles incoming data.
 * The main routine (run()) runs in a separate thread to allow the main program to continue its execution.
 * On each incoming data it:
 * 		1. Respond with "OK"
 * 		2. Call the callback function with the incoming data (as string)
 * 		3. Add the incoming data to an internal queue
 */
class TCPServer {
public:
	/**
	 * Constructor.
	 * Receives port as integer and a callback function as callback_t type.
	 */
	TCPServer(int, callback_t);
	/**
	 * Copy constructor.
	 */
	TCPServer(const TCPServer &other);
	/**
	 * Assignment Operator.
	 */
	TCPServer& operator=(const TCPServer &other);
	/**
	 * Destructor.
	 */
	virtual ~TCPServer();

	/**
	 * Getters for port and callback function
	 */
	int get_port() const;
	callback_t get_callback() const;

	/**
	 * Public function to pop data from the internal queue.
	 */
	string pop_data();

	/**
	 * Starts the internal thread that executes the main routine (run()).
	 */
	void start();

	/**
	 * Stops the main routine and the internal thread.
	 */
	void stop();

private:
	//Local Variables
	int _port;					//Listener port
	callback_t _callback;		//Pointer to the callback function
	bool _running;				//Flag for starting and terminating the main loop
	queue<string>* _data;		//Queue for saving incoming data
	thread _thread;				//Internal thread, this is in order to start and stop the thread from different class methods

	/**
	 * This method handles incoming data. Calling callback function and adding data to queue.
	 */
	void handler(string);

	/**
	 * This is the main routine of this class.
	 * It accepts incoming connection and receives incoming data from these connections.
	 * It is private because it is only executed in a different thread by start() method.
	 */
	void run();

};

#endif /* TCPSERVER_H_ */
