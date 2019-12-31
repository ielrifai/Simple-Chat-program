#include "thread.h"
#include "socketserver.h"
#include <stdlib.h>
#include <algorithm>
#include <time.h>
#include <list>
#include <vector>
#include <string.h>
#include <stdio.h>
#include "Semaphore.h"
#include <string>

using namespace Sync;

//User class
//Stores username and specified chatroom ID of the client
// More importantly - stores a reference to each client's socket to allow communication between clients
 class User 
 {
     public:
     std::string username;
     Socket& socket;
     std::string chatRoomID;
    
     User(Socket& socket, std::string username) : socket(socket), username(username)
     {
         chatRoomID = "";
     }
 };
std::vector<User*> allUsers; //vector of sockets

// This thread handles each client connection
class SocketThread : public Thread
{
private:
    // Reference to our connected socket
    Socket& socket;

    // The data we are receiving
    ByteArray data;
    
    std::string recepient;
    bool& terminate;
    int port;
    
    std::vector<SocketThread*> &clientSocketThreads;
    
    int chatRoom;
public:
    SocketThread(Socket& socket, bool& terminate, int port, std::vector<SocketThread*> &clientSocketThreads)
    : socket(socket), terminate(terminate), port(port), clientSocketThreads(clientSocketThreads)
    {
        recepient = "";
    }

    ~SocketThread()
    {}

    Socket& GetSocket()
    {
        return socket;
    }
    
    const int GetChatRoom(){
        return chatRoom;
    }

    virtual long ThreadMain()
    {
        std::string portstr = std::to_string(port);
        Semaphore protect(portstr);
        // If terminate is ever flagged, we need to gracefully exit
        while(!terminate)
        {
            try
            {
                // Wait for data
                //socket.Read(data);
                if (socket.Read(data) > 0)
                {
    
                    
                    // Transforms the data to a string and splits it between the command identifiers & the actual data
                    std::string data_str = data.ToString();
                    std::cout << data_str <<std::endl;
                    std::string newString = data.ToString();
                    std::string newString2 = data.ToString();
                    std::string token = newString.substr(0, newString.find("/")); // "/" is used to split the identifier from the data
                    std::string userInput;
                    std::string str = data.ToString(); 
                    
                    std::cout << str << std::endl;        
                  
                    //Parsing string
                    std::string unit, mystring(data.ToString());
                    while(unit != mystring){
                      unit = mystring.substr(0,mystring.find_first_of("/"));
                      mystring = mystring.substr(mystring.find_first_of("/") + 1);
                      userInput = unit.c_str();
                    }
					//Initial connection message by the client
                    if (token == "initial")
                    {
                        User* newClient = new User(socket, userInput); //Want to push user onto a vector stack
                        allUsers.push_back(newClient);
                        std::vector<User*>::iterator user;
                         for(auto user = allUsers.begin(); user != allUsers.end(); user++) {
                             std::cout << (*user)->username<< std::endl;
                         }
                    }
					//When the user wants to specify a chatroom
                    else if (token == "chatRoom")
                    {
                        //Need to parse userInput to get /chatRoom/userName/chatRoomID
                        std::string originalUserName; //Need the user name of the client in order to assign a chat room to it
                        std::string _unit2, _mystring2(data.ToString());
                        for (int i = 0; i < 2; i++)
                        {
                            _unit2 = _mystring2.substr(0,_mystring2.find_first_of("/"));
                            _mystring2 = _mystring2.substr(_mystring2.find_first_of("/") + 1);
                            originalUserName = _unit2.c_str();
                        }
                        std::cout << "Original user name is: " << originalUserName << std::endl;
                        
                        //Set the chat room for the user
                        std::cout << "ChatRoom in server has been initialized " << std::endl;
                        std::cout << "userInput sent from the client is " + userInput << std::endl;
                        std::vector<User*>::iterator user;
                        std::string orignalSenderChatRoom;
                        for(auto user = allUsers.begin(); user != allUsers.end(); user++) { //Assign chat room to the corresponding user
                                 if ((*user)->username == originalUserName)
                                 {
                                     (*user)->chatRoomID = userInput;
                                     orignalSenderChatRoom = userInput;
                                 }
                            }
                        //Tell user if anyone is in the chatroom
                        bool foundUsers = false;
                        for(auto user = allUsers.begin(); user != allUsers.end(); user++) { //Find all users in the chatroom
                                 if ((*user)->username != originalUserName && ((*user)->chatRoomID == orignalSenderChatRoom))//If not the same user and they share the same chatroomID
                                 {
									 std::cout << "Users found in chatroom" << std::endl;
                                     socket.Write(ByteArray("User " + (*user)->username + " is in this chatroom\n"));
                                     foundUsers = true;
                                 }
                            }
                        if (foundUsers == false)
                        {
							std::cout << "No users in chatroom" << std::endl;
                            socket.Write(ByteArray("No other users are currently in this chat room\n"));
                        }
                    }
					//Client sends a termination message before closing
                    else if (token == "terminateClient")
                    {
                        socket.Write(ByteArray("terminateClient/\n"));
                        int counter = 0;
                        //Delete client from user vector
                        std::vector<User*>* currentUsers = &allUsers;
                        
                        std::vector<User*>::iterator user;
                        for(auto user = allUsers.begin(); user != allUsers.end(); user++) { //Find all users in the chatroom
                            counter++;
                            if ((*user)->username == userInput)
                            {
                                std::cout << "Iterator value: " + std::to_string(counter) << std::endl;
                                currentUsers->erase(user);
                                break;
                            }
                        }
                        std::cout << "Test" << std::endl;
                        
                        
                    }
					//For regular chatting messages
                    else{
                        std::string matchingChatRoom = "";
                        std::string originalSender = "";
                        std::vector<User*>::iterator user;
                        for(auto user = allUsers.begin(); user != allUsers.end(); user++) {
                            
                            if ((*user)->username == token) //Token will be the username of the client that is sending
                            {
                                //Find the chatroom belonging to that client
                                matchingChatRoom = (*user)->chatRoomID;
                                originalSender = (*user)->username;
                                std::cout << "Matching chat room: " << matchingChatRoom << std::endl;
                                std::cout << "Original sender: " << originalSender << std::endl;
                            }
                        }
                        
                         //Find all users in that chatroom
                         for(auto user = allUsers.begin(); user != allUsers.end(); user++) {
                            if ((*user)->chatRoomID == matchingChatRoom && ((*user)->username != originalSender)) //Token will be the username of the client that is sending
                            {
                                //Write to all user sockets in that chat room
                                (*user)->socket.Write(ByteArray(originalSender + ": " + userInput+"\n"));
                            }
                        }
                        
                        
                    }
                } else 
                {
                    break;
                }
            }
            catch (...)
            {
                // We catch the exception, but there is nothing for us to do with it here. Close the thread.
            }
        }
        if (terminate) {
			std::vector<User*>* currentUsers = &allUsers;

			std::vector<User*>::iterator user;
			for (auto user = allUsers.begin(); user != allUsers.end(); user++) { //Find all users in the chatroom
					currentUsers->erase(user);
			}
        }
        return 0;
    }
};

// This thread handles the server operations
class ServerThread : public Thread
{
private:
    SocketServer& server;
    std::vector<SocketThread*> socketThreads; //vector of sockets
    bool terminate = false;
    int port;
    int chatNum;
public:
    ServerThread(SocketServer& server)
    : server(server), port(port), chatNum(chatNum)
    {}

    ~ServerThread()
    {
        // Close the client sockets
        for (auto thread : socketThreads)
        {
            try
            {
                // Close the socket
                Socket& toClose = thread->GetSocket();
                toClose.Close();
            }
            catch (...)
            {
                // If already closed, this will cause an exception
            }
        }

        // Terminate the thread loops
        terminate = true;
    }

    virtual long ThreadMain()
    {
        while(true)
        {
            try
            {
                std::string portstr = std::to_string(port);
                std::cout << "Waiting for a client" <<std::endl;
                Semaphore protect(portstr, 1, true);
                std::string numberOfChats = std::to_string(chatNum) + '\n';
                ByteArray ba(numberOfChats);
                
                
                // Wait for a client socket connection
                Socket* newConnection = new Socket(server.Accept());
                sock.Write(ba);
                // Pass a reference to this pointer into a new socket thread
                Socket& socketReference = *newConnection;
                socketThreads.push_back(new SocketThread(socketReference, terminate));
            }
            catch (TerminationException terminationException)
            {
                return terminationException;
            }
            catch (std::string error)
            {
                std::cout << std::endl << "[Error] " << error << std::endl;
                return 1;
            }
        }
    }
};

int main(void)
{
    
    // Welcome the user
    std::cout << "SE3313 Lab 4 Server" << std::endl;
    std::cout << "Press enter to terminate the server...";
    std::cout.flush();

    // Create our server
    SocketServer server(3002);    

    // Need a thread to perform server operations
    ServerThread serverThread(server);

    // This will wait for input to shutdown the server
    FlexWait cinWaiter(1, stdin);
    cinWaiter.Wait();
    std::cin.get();
	std::vector<User*>* currentUsers = &allUsers;

	std::vector<User*>::iterator user;
	for (auto user = allUsers.begin(); user != allUsers.end(); user++) { //Find all users in the chatroom
		(*user)->socket.Close();
		currentUsers->erase(user);
	}
    // Shut down and clean up the server
    server.Shutdown();
    return 0;
}
