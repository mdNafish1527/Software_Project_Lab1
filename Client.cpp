#include <iostream>
#include <cstring>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>

#define PORT 8080
#define BUFFER_SIZE 1024

using namespace std;

int main(void) 
{
    int clientSocket;
    struct sockaddr_in serverAddress;
    char buffer[BUFFER_SIZE] = {0};

    if ((clientSocket = socket(AF_INET, SOCK_STREAM, 0)) < 0) 
    {
        cout << "Your connection is not cirrect please check out" << endl;
        return -1;
    }

    serverAddress.sin_family = AF_INET;
    serverAddress.sin_port = htons(PORT);

    if (inet_pton(AF_INET, "127.0.0.1", &serverAddress.sin_addr) == 0 || inet_pton(AF_INET, "127.0.0.1", &serverAddress.sin_addr) < 0) 
    {
        cout << "Your current ip address not supported" << endl;
        return -1;
    }

    if (connect(clientSocket, (struct sockaddr*)&serverAddress, sizeof(serverAddress)) < 0) 
    {
        cout << "loose connection" << endl;
        return -1;
    }

    while (true) 
    {
        cout << "Enter command (register/login/send_mail/Mailbox/logout/exit): ";
        string command, username, password, recipient, massage;
        cin >> command;

        string request = command;
        if (command == "register" || command == "login") 
        {
            cout << "Enter username: ";
            cin >> username;
            cout << "Enter password: ";
            cin >> password;
            request += ":" + username + ":" + password;
        } 
        else if (command == "send_mail") 
        {
            cout << "Enter recipient: ";
            cin >> recipient;
            cin.ignore();
            cout << "Enter mail content: ";
            getline(cin, massage);
            request += ":" + recipient + ":" + massage;
        }

        send(clientSocket, request.c_str(), request.length(), 0);

        memset(buffer, 0, BUFFER_SIZE);
        read(clientSocket, buffer, BUFFER_SIZE);
        cout << "Server: " << buffer;

        if (command == "exit") 
        {
            break;
        }
    }

    close(clientSocket);
    return 0;
}
