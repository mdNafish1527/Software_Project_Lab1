#include <bits/stdc++.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#define PORT 8080
#define BUFFER_SIZE 1024
using namespace std;

int main(void) 
{
    int my_socket;
    struct sockaddr_in server_adress;
    char new_buffer[BUFFER_SIZE] = {0};

    if ((my_socket = socket(AF_INET, SOCK_STREAM, 0)) < 0) 
    {
        cout << "Your connection is not cirrect please check out" << endl;
        return -1;
    }

    server_adress.sin_family = AF_INET;
    server_adress.sin_port = htons(PORT);

    if (inet_pton(AF_INET, "127.0.0.1", &server_adress.sin_addr) == 0 || inet_pton(AF_INET, "127.0.0.1", &server_adress.sin_addr) < 0) 
    {
        cout << "Your current ip address not supported" << endl;
        return -1;
    }

    if (connect(my_socket, (struct sockaddr*)&server_adress, sizeof(server_adress)) < 0) 
    {
        cout << "loose connection" << endl;
        return -1;
    }

    while (true) 
    {
        cout << "Enter your command (register/login/send_mail/Mailbox/logout/exit): ";
        string comand, username, passsword, recipient, massage;
        cin >> comand;

        string request = comand;
        if (comand == "register" || comand == "login") 
        {
            cout << "Enter username: ";
            cin >> username;
            cout << "Enter password: ";
            cin >> passsword;
            request += ":" + username + ":" + passsword;
        } 
        else if (comand == "send_mail") 
        {
            cout << "Enter recipient: ";
            cin >> recipient;
            cin.ignore();
            cout << "Enter mail content: ";
            getline(cin, massage);
            request += ":" + recipient + ":" + massage;
        }

        send(my_socket, request.c_str(), request.length(), 0);

        memset(new_buffer, 0, BUFFER_SIZE);
        read(my_socket, new_buffer, BUFFER_SIZE);
        cout << "Server: " << new_buffer;

        if (comand == "exit") 
        {
            break;
        }
    }

    close(my_socket);
    return 0;
}
