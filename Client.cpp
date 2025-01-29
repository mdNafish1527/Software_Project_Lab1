#include <iostream>
#include <string>
#include <unistd.h>
#include <arpa/inet.h>
#include <cstring>

using namespace std;


void sendData(int sock, const string& data) 
{
    int length = data.length();
    char lenBuffer[4];
    int networkLength = htonl(length); 
    memcpy(lenBuffer, &networkLength, 4); 
    send(sock, lenBuffer, 4, 0); 
    send(sock, data.c_str(), length, 0); 
}


int main(void) 
{
    int sock;
    struct sockaddr_in serverAddr;
    char buffer[1024];
    string command, username, password, recipient, message;


    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) == -1) 
        {
            cerr << "Socket creation failed" << endl;
            return 1;
        }

    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(8080);
    serverAddr.sin_addr.s_addr = inet_addr("127.0.0.1");

    
    if (connect(sock, (struct sockaddr*)&serverAddr, sizeof(serverAddr)) == -1) 
        {
            cerr << "Connection to server failed" << endl;
            return 1;
        }

    cout << "Connected to the server!" << endl;

    while (true) 
    {
        cout << "Enter command (register, login, sendmail, viewmail, or quit): ";
        cin >> command;

        if (command == "register") 
            {
                cout << "Enter username: ";
                cin >> username;
                cout << "Enter password: ";
                cin.ignore();
                getline(cin, password);

                sendData(sock, "register");
                sendData(sock, username);
                sendData(sock, password);

                recv(sock, buffer, sizeof(buffer), 0);
                cout << buffer << endl;
            } 
        else if (command == "login") 
            {
                cout << "Enter username: ";
                cin >> username;
                cout << "Enter password: ";
                cin.ignore(); 
                getline(cin, password); 

                sendData(sock, "login");
                sendData(sock, username);
                sendData(sock, password);

                recv(sock, buffer, sizeof(buffer), 0);
                cout << buffer << endl;
            } 
        else if (command == "sendmail") 
            {
                cout << "Enter recipient's username: ";
                cin >> recipient;
                cout << "Enter your message: ";
                cin.ignore(); 
                getline(cin, message);

                sendData(sock, "sendmail");
                sendData(sock, recipient);
                sendData(sock, message);

                recv(sock, buffer, sizeof(buffer), 0);
                cout << buffer << endl;
            } 
        else if (command == "viewmail") 
            {
                sendData(sock, "viewmail");
                recv(sock, buffer, sizeof(buffer), 0);
                cout << buffer << endl;
            } 
        else if (command == "quit") 
            {
                break;
            } 
        else 
            {
                cout << "Invalid command" << endl;
            }
    }

    close(sock);
    return 0;
}
