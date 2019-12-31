#include "thread.h"
#include "socket.h"
#include <iostream>
#include <stdlib.h>
#include <time.h>
#include <sstream>

using namespace Sync;

//Need to assign a unique identifier to each client
int userID = 0;
std::string recipient;
std::string chatRoomID;

class ReaderThread : public Thread {
	private:
	// Reference to our connected socket
	Socket& socket;

	bool& terminate;
	ByteArray data;
	public:
	ReaderThread(Socket& socket, bool& terminate)
	: socket(socket), terminate(terminate)
	{

	}

	~ReaderThread()
	{}
	virtual long ThreadMain(){
		while(!terminate){
			try{
				if(socket.Read(data) >0 ){
					
					std::string data_str = data.ToString();
					if (data_str == "terminateClient/")
					{
						break;
					}
					std::cout << data_str << std::endl;
				}

			}catch (...) {

            }
		}
	}
};


// This thread handles the connection to the server

//NOTE NEED TO CHANGE HOW READER THREAD IS CREATED TO BE CALLED WHEN CLIENTTHREAD IS CREATED
class ClientThread : public Thread
{
private:
	// Reference to our connected socket
	Socket& socket;

	// Reference to boolean flag for terminating the thread
	bool& terminate;
	
	//Counter to determine if first message
	int messageCount;
	std::string username;


	// Are we connected?
	bool connected = false;
	
	// Data to send to server
	ByteArray data;
	std::string data_str;
	int expectedLength = 0;
public:
	ClientThread(Socket& socket, bool& terminate)
	: socket(socket), terminate(terminate)
	{
		messageCount = 0;
		recipient = "";
		username="";
	}

	~ClientThread()
	{}

	void TryConnect()
	{
		try
		{
			std::cout << "Connecting...";
			std::cout.flush();
			socket.Open();
			connected = true;
			std::cout << "OK" << std::endl;
		}
		catch (std::string exception)
		{
			std::cout << "FAIL (" << exception << ")" << std::endl;
			return;
		}
	}

	virtual long ThreadMain()
	{
		// Initially we need a connection
		while (true)
		{
			// Attempt to connect
			TryConnect();

			// Check if we are exiting or connection was established
			if (terminate || connected)
			{
				break;
			}

			// Try again every 5 seconds
			std::cout << "Trying again in 5 seconds" << std::endl;
			sleep(5);
		}

		while (!terminate)
		{
			//Please specify who this message is to
			
			//Please specify your message
			if (messageCount == 0) //Want to create a new user in the server
			{
				std::cout << "Please enter your username " << std::endl;
				std::getline(std::cin, username);
				//Create user and add to vector in server
				socket.Write(ByteArray("initial/" + username));
				
				//Please specify the chat room
				std::cout << "Please enter what chat room you want to enter " << std::endl;
				std::getline(std::cin, chatRoomID);
				//Initialize chat room for this client in the server
				socket.Write(ByteArray("chatRoom/" + username + "/" + chatRoomID));
				std::cout << "Connected to chatRoom : " + chatRoomID + " . Enter done at any time to exit." << std::endl;
				messageCount++;
				
			}
			else {
			
				// We are connected, perform our operations
				std::cout.flush();
	
				// Get the data
				data_str.clear();
				std::getline(std::cin, data_str);
				data = ByteArray(username + "/"+ data_str); //Send the username 
				
				expectedLength = data_str.size();
	
				// Must have data to send
				if (expectedLength == 0)
				{
					std::cout << "Cannot send no data!" << std::endl;
					continue;
				}
				else if (data_str == "done")
				{
					std::cout << "Closing the client..." << std::endl;
					
					socket.Write(ByteArray("terminateClient/" + username));
					sleep(5);
					terminate = true;
					break;
				}
				// Both Write/Read return the number of bytes sent/received
				if (socket.Write(data) <= 0)
				{
					std::cout << "Server failed to respond. Closing client..." << std::endl;
					terminate = true;
				}
			}
		}

		return 0;
	}
};

int main(void)
{
	// Welcome the user and try to initialize the socket
	std::cout << "SE3313 Lab 4 Client" << std::endl;

	// Create our socket
	Socket socket("35.162.177.130", 3002);
	//Socket socket("35.162.177.130", 3000);
	//Socket socket("35.162.177.130", 3000);
	bool terminate = false;

	// Scope to kill thread
	{
		// Thread to perform socket operations on
		ClientThread clientThread(socket, terminate);
		ReaderThread readerThread(socket, terminate);
		while(!terminate)
		{
			// This will wait for 'done' to shutdown the server
			sleep(1);
		}
		
		// Wait to make sure the thread is cleaned up
	}
	
	// Attempt to close the socket
	try
	{
		socket.Close();
	}
	catch (...)
	{
		// We don't care if this failed because the application is exiting anyways.
	}

	return 0;
}
