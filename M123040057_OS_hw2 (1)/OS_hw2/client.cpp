#include <iostream>
#include <cstring>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

using namespace std;

int main()
{
    int sockfd;
    struct sockaddr_in server_addr;
    char rcv_buffer[1000];
    char send_buffer[1000];
    sockfd = socket(AF_INET, SOCK_STREAM, 0);

    // Initialize the server information
    memset(&server_addr, 0, sizeof(server_addr)); //set all the bytes in the memory block starting at the address of serv_addr to zero.
    /* set up the server info */
    server_addr.sin_family = AF_INET;	
    server_addr.sin_addr.s_addr = inet_addr("127.0.0.1");	
    server_addr.sin_port = htons(7000);	

    // Connect to the server
    connect(sockfd, (struct sockaddr *)&server_addr, sizeof(server_addr));
    recv(sockfd, rcv_buffer, 1000, 0);
    cout << " User: " << rcv_buffer << endl;

    // 持續接收client指令
    while (true) {  
        memset(rcv_buffer, 0, sizeof(rcv_buffer));
        memset(send_buffer, 0, sizeof(send_buffer));

        cout << "Enter your command: ";
        string input;
        getline(cin, input);

   

        // Convert string to c-string array
        strncpy(send_buffer, input.c_str(), sizeof(send_buffer) - 1);
        send(sockfd, send_buffer, sizeof(send_buffer), 0);  // send message to the client
        recv(sockfd, rcv_buffer, sizeof(rcv_buffer), 0);  // receive message from the server

        cout << "Server message: " << string(rcv_buffer) << endl;
    }

}
