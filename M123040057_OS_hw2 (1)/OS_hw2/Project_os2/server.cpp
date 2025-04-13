#include <iostream>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <cstring>
#include <unistd.h>
#include <sys/types.h> 
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <vector> 
#include <sstream> 
#include <fstream>
#include <map>
#include <algorithm>

using namespace std;

// Define a mutex for thread safety when accessing the file repository
pthread_mutex_t repositoryMutex = PTHREAD_MUTEX_INITIALIZER;
int usr_count=0;
int file_num=0;

// Define a structure to store file information
struct File {
    string name;
    string owner;
    string group;
    string content; // Added content field
    struct Permissions {
        char ownerRead;
        char ownerWrite;
        char groupRead;
        char groupWrite;
        char othersRead;
        char othersWrite;
    } permissions;
    bool canbeRead;
    bool canbeWrite;
};

vector<File> fileRepository;

// Define a structure to store client information
struct Client {
    int socket;
    string username;
    string group;
};

void file_info(){
    cout<< "--------File Information--------"<<endl;
    for(int i=0;i<file_num;i++){
        cout<< "Filename: " << fileRepository[i].name << " | Owner: "<< fileRepository[i].owner <<
        " | Group: "<< fileRepository[i].group <<
        " | Capability List : " 
        << fileRepository[i].permissions.ownerRead
        << fileRepository[i].permissions.ownerWrite
        << fileRepository[i].permissions.groupRead
        << fileRepository[i].permissions.groupWrite
        << fileRepository[i].permissions.othersRead
        << fileRepository[i].permissions.othersWrite << endl;
    }

}


void addFileToRepository(Client *client, const string &filename, const string &permissions) {
    // Assuming the file is in the /home/poyu/os_hw2/ directory
    string fullPath = "/home/poyu/os_hw2/" + filename;
    FILE *file = fopen(fullPath.c_str(), "r");
    
    //pthread_mutex_lock(&repositoryMutex);
    
    ifstream inFile(fullPath);
    stringstream contentStream;
    contentStream << inFile.rdbuf();
    //string fileContent = contentStream.str();
    
    string fileContent = client->username + " " + fullPath; 
    
    if (file != nullptr) {
        
        File newFile{
            filename,
            client->username, // Placeholder for owner, as it is not known in this context
            client->group, // Placeholdedr for group, as it is not known in this context
            fileContent, // Initialize content to empty
            {permissions[0], permissions[1], permissions[2], permissions[3], permissions[4], permissions[5]},
            true,
            true
        };

        fileRepository.push_back(newFile);
        //pthread_mutex_unlock(&repositoryMutex);

        cout << "File " << filename << " added to fileRepository." << endl;
    } else {
        cout << "Error opening file " << filename << ". File not added to fileRepository." << endl;
    }
    
     //pthread_mutex_unlock(&repositoryMutex);
}



void handleCreate(Client *client, const string &filename, const string &permissions) {
    
    cout << "Received create command. Filename: " << filename << ", Permissions: " << permissions << endl;
     
    string fullPath = "/home/poyu/os_hw2/" + filename;
    FILE *file = fopen(fullPath.c_str(), "w");
    if (file == nullptr) {
    perror("Error opening file");
    send(client->socket, "Error opening file.", 20, 0);  
    return;
}
     
    
    pthread_mutex_lock(&repositoryMutex);
    addFileToRepository(client, filename, permissions);
    send(client->socket, "File created successfully.", 26, 0);
    file_num++;
    pthread_mutex_unlock(&repositoryMutex);
}


// Function to handle the "read" command
void handleRead(Client *client, const string &filename, const string &permissions) {
    
    cout << "Received read command. Filename: " << filename << endl;
    //addFileToRepository(client, filename, permissions);
    
    addFileToRepository(client, filename, permissions);
     //string fullPath = "/home/poyu/os_hw2/" + filename;
    //FILE *file = fopen(fullPath.c_str(), "r+");
    //bool fileExists = ifstream(fullPath.c_str()).good();
    auto it = find_if(fileRepository.begin(), fileRepository.end(),
                      [&filename](const File &file) { return file.name == filename; });

    pthread_mutex_lock(&repositoryMutex);
    if (it != fileRepository.end()) {
    	if(it->permissions.ownerRead=='r' && client->username==it->owner && it->canbeRead){
    	   it->canbeWrite == false;
    	   for(int i=0;i<1000000000;i++); // sleep for 10 seconds
    	   it->canbeWrite == true;
           send(client->socket, "Read finsih", 12, 0);
        }
        else if(it->permissions.groupRead=='r' && client->group==it->group && it->canbeRead){
           it->canbeWrite == false;
    	   for(int i=0;i<1000000000;i++);  // sleep for 10 seconds
    	   it->canbeWrite == true;
           send(client->socket, "Read finsih", 12, 0);
        }
        else if(it->permissions.othersRead=='r' && client->group!=it->group && it->canbeRead){
           it->canbeWrite == false;
    	   for(int i=0;i<1000000000;i++);  // sleep for 10 seconds
    	   it->canbeWrite == true;
           send(client->socket, "Read finsih", 12, 0);
        }
        else{
           send(client->socket, "Permission denied.", 19, 0);
        }
    }
    else {
        send(client->socket, "File not found.", 15, 0);
    }
    pthread_mutex_unlock(&repositoryMutex);
}


// Function to handle the "write" command
void handleWrite(Client *client, const string &filename, const string &writeMode) {
    pthread_mutex_lock(&repositoryMutex);
    string fullPath = "/home/poyu/os_hw2/" + filename;
    auto it = find_if(fileRepository.begin(), fileRepository.end(),
                      [&filename](const File &file) { return file.name == filename; });

    if (it != fileRepository.end()) {
    	if(it->permissions.ownerWrite=='w' && client->username==it->owner && it->canbeWrite){
    	   if (writeMode.compare("o")==0){
    	       it->canbeRead == false;
    	       it->canbeWrite == false;
    	       for(int i=0;i<1000000000;i++);
    	       it->canbeRead == true;
    	       it->canbeWrite == true;
    	       ofstream outfile(fullPath);
    	       outfile << "This is new overwrited content by "<< client->username << "."<<endl;
    	   }
    	   else if (writeMode.compare("a")==0){
    	       it->canbeRead == false;
    	       it->canbeWrite == false;
    	       for(int i=0;i<1000000000;i++);
    	       it->canbeRead == true;
    	       it->canbeWrite == true;
    	       ofstream outfile(fullPath, ios::app);
    	       outfile << "This is new additional content by "<< client->username << "."<<endl;
    	   }
           send(client->socket, "Write finsih", 12, 0);
        }
        else if(it->permissions.groupWrite=='w' && client->group==it->group && it->canbeWrite){
           if (writeMode.compare("o")==0){
               it->canbeRead == false;
    	       it->canbeWrite == false;
    	       for(int i=0;i<1000000000;i++);
    	       it->canbeRead == true;
    	       it->canbeWrite == true;
    	       ofstream outfile(fullPath);
    	       outfile << "This is new overwrited content by "<< client->username << "."<<endl;
    	   }
    	   else if (writeMode.compare("a")==0){
    	       it->canbeRead == false;
    	       it->canbeWrite == false;
    	       for(int i=0;i<1000000000;i++);
    	       it->canbeRead == true;
    	       it->canbeWrite == true;
    	       ofstream outfile(fullPath, ios::app);
    	       outfile << "This is new additional content by "<< client->username << "."<<endl;
    	   }
           send(client->socket, "Write finsih", 12, 0);
        }
        else if(it->permissions.othersWrite=='w' && client->group!=it->group && it->canbeWrite){
           if (writeMode.compare("o")==0){
               it->canbeRead == false;
    	       it->canbeWrite == false;
    	       for(int i=0;i<1000000000;i++);
    	       it->canbeRead == true;
    	       it->canbeWrite == true;
    	       ofstream outfile(fullPath);
    	       outfile << "This is new overwrited content by "<< client->username << "."<<endl;
    	   }
    	   else if (writeMode.compare("a")==0){
    	       it->canbeRead == false;
    	       it->canbeWrite == false;
    	       for(int i=0;i<1000000000;i++);
    	       it->canbeRead == true;
    	       it->canbeWrite == true;
    	       ofstream outfile(fullPath, ios::app);
    	       outfile << "This is new additional content by "<< client->username << "."<<endl;
    	   }
           send(client->socket, "Write finsih", 12, 0);
        }
        else{
           send(client->socket, "Permission denied.", 19, 0);
        }
    }
    else {
        send(client->socket, "File not found.", 15, 0);
    }
    pthread_mutex_unlock(&repositoryMutex);
}


void handleChangeMode(Client *client, const string &filename, const string &newPermissions) {
    auto it = find_if(fileRepository.begin(), fileRepository.end(),
                      [&filename](const File &file) { return file.name == filename; });

    pthread_mutex_lock(&repositoryMutex);
    if (it != fileRepository.end() && it->name==filename) {
    	it->permissions.ownerRead = newPermissions[0];
    	it->permissions.ownerWrite = newPermissions[1];
    	it->permissions.groupRead = newPermissions[2];
    	it->permissions.groupWrite = newPermissions[3];
    	it->permissions.othersRead = newPermissions[4];
    	it->permissions.othersWrite = newPermissions[5];
    	send(client->socket, "Mode has been changes.", 23, 0);
    }
    else {
        send(client->socket, "File not found.", 15, 0);
    }
    pthread_mutex_unlock(&repositoryMutex);
}



void handleClientCommand(Client *client, const string &command) {
    cout<<"----Command Get----"<<endl;
    if (command.substr(0, 6) == "create") {
        size_t spaceIndex = command.find(" ");
        string filename = command.substr(spaceIndex + 1, command.find(" ", spaceIndex + 1) - spaceIndex - 1);
        string permissions = command.substr(spaceIndex + 1 + filename.size() + 1);
        handleCreate(client, filename, permissions);
    } 
    
    else if (command.substr(0, 4) == "read") {
        size_t spaceIndex = command.find(" ");
        string filename = command.substr(spaceIndex + 1, command.find(" ", spaceIndex + 1) - spaceIndex - 1);
        string permissions = "";
        handleRead(client, filename, permissions);
    } 
    
    else if (command.substr(0, 5) == "write") {
        size_t spaceIndex = command.find(" ");
        string filename = command.substr(spaceIndex + 1, command.find(" ", spaceIndex + 1) - spaceIndex - 1);
        string writeMode = command.substr(spaceIndex + 1 + filename.size() + 1);
        handleWrite(client, filename, writeMode);
    } 
    
    else if (command.substr(0, 10) == "changemode") {
        size_t spaceIndex = command.find(" ");
        string filename = command.substr(spaceIndex + 1, command.find(" ", spaceIndex + 1) - spaceIndex - 1);
        string newPermissions = command.substr(spaceIndex + 1 + filename.size() + 1);
        handleChangeMode(client, filename, newPermissions);
    } 
    
    else {
        send(client->socket, "Invalid command.", 17, 0);
    }
}

// Modified handleClient function
void *handleClient(void *arg) {
    Client *client = static_cast<Client *>(arg);
    char rcv_buffer[1000];
    char send_buffer[1000];
    
    string name;
    int group = usr_count % 2; //0 is AOS 1 is CSE
    if(group)
    {
        name = "CSE-" + to_string(usr_count - 1);
        client->group = "CSE";
    }
    else
    {
        name = "AOS-" + to_string(usr_count);
        client->group = "AOS";
    }
    cout << "user" << name <<" joined" << endl;
    client->username = name;
    strcpy(send_buffer, name.c_str()); //convert to c str array
    usr_count++;
    send(client->socket, send_buffer, sizeof(send_buffer), 0);

    while (true) {
        memset(rcv_buffer, 0, sizeof(rcv_buffer));
        //recv(client->socket, rcv_buffer, sizeof(rcv_buffer), 0);
        ssize_t bytesReceived = recv(client->socket, rcv_buffer, sizeof(rcv_buffer), 0);

        if (bytesReceived <= 0) {
            // Connection closed or error, break the loop
            break;
        }

        string command(rcv_buffer);

        // Handle command using the dedicated functions
        handleClientCommand(client, command);
        file_info();
    }

    close(client->socket);
    //delete client;
    pthread_exit(NULL);
}


int main() {
    int serverSocket;
    socklen_t addrlen;
    struct sockaddr_in serverAddr, clientAddr;
    char rcv_buffer[1000];
    char send_buffer[1000];

    // Initialize the server socket
    serverSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (serverSocket < 0) {
        perror("ERROR opening socket.");
        exit(EXIT_FAILURE);
    }

    // Initialize the server address
    memset(&serverAddr, 0, sizeof(serverAddr));
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_addr.s_addr = inet_addr("127.0.0.1");
    serverAddr.sin_port = htons(7000);

    // Bind the server socket to the server address
    if (bind(serverSocket, (struct sockaddr *)&serverAddr, sizeof(serverAddr)) < 0) {
        perror("ERROR on binding.");
        exit(EXIT_FAILURE);
    }

    // Listen for incoming connections
    listen(serverSocket, 20);    

    cout << "Server is listening on port 7000..." << endl;
    
    
    memset(&clientAddr, 0, sizeof(clientAddr));

    // Accept incoming connections and handle commands
    while (true) {
    	addrlen = sizeof(clientAddr);  // Add this line to initialize addrlen
        // Accept a connection from a client
        int clientSocket = accept(serverSocket, (struct sockaddr *)&clientAddr, &addrlen);

	    Client *clientInfo = new Client{clientSocket, "username", "group"};

        // Create a new thread to handle the client
        pthread_t clientThread;
        pthread_create(&clientThread, NULL, handleClient, clientInfo);
    }

    return 0;
}
