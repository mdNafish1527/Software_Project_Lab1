#include <iostream>
#include <string>
#include <unistd.h>
#include <arpa/inet.h>
#include <cstring>
using namespace std;





string atbashChipperEncrypt(const string& input)
    {
        string output;
        for(int i = 0 ;i < input.size() ; i++)
        {
            output.push_back(input[i]);
        }

        for(int i = 0 ; i < output.size() ; i++)
        {
            if(isalpha(output[i]))
                {
                    if(output[i] >= 'A' && output[i] <= 'Z')
                        {
                            output[i] = 'Z' + 'A' - output[i];
                        }
                        else if(output[i] >= 'a' && output[i] <= 'z')
                        {
                            output[i] = 'z' + 'a' - output[i];
                        }
                        else if(output[i] >= '0' && output[i] <= '9')
                        {
                            output[i] = '9' - output[i];
                        }
                }
        }
        return output;
    }



string atbashChipperDecrypt(const string& input)
    {
        string output;
        for(int i = 0 ;i < input.size() ; i++)
        {
            output.push_back(input[i]);
        }

        for(int i = 0 ; i < output.size() ; i++)
        {
            if(isalpha(output[i]))
                {
                    if(output[i] >= 'A' && output[i] <= 'Z')
                        {
                            output[i] = 'Z' + 'A' - output[i];
                        }
                    else if(output[i] >= 'a' && output[i] <= 'z')
                        {
                            output[i] = 'z' + 'a' - output[i];
                        }
                        else if(output[i] >= '0' && output[i] <= '9')
                        {
                            output[i] = '9' - output[i];
                        }
                }
        }
        return output;
        
    }


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
    string command, username, password, recipient, message, newPass;


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
        cout << "Enter command (register, login, sendmail, viewmail,changepass or quit): ";
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
        else if (command == "changepass")
            {
                cout << "Enter username ";
                cin >> username;
                cout << "Enter old password ";
                cin.ignore();
                getline(cin,password);
                
                cout << "ENter new Password ";
                getline(cin,newPass);


                sendData(sock,"changepass");
                sendData(sock,username);
                sendData(sock,password);
                sendData(sock,newPass);

                recv(sock,buffer,sizeof(buffer),0);
                cout << buffer << endl;
            }
        else if (command == "sendmail") 
            {
                cout << "Enter recipient's username: ";
                cin >> recipient;
                cout << "Enter your message: ";
                cin.ignore(); 
                getline(cin, message);


                for(int i = 0 ; i < message.size() ; i++)
                    {
                        if(isalpha(message[i]))
                            {
                                if(message[i] >= 'A' && message[i] <= 'Z')
                                    {
                                        message[i] = 'Z' + 'A' - message[i];
                                    }
                                else if(message[i] >= 'a' && message[i] <= 'z')
                                    {
                                        message[i] = 'z' + 'a' - message[i];
                                    }
                                    else if(message[i] >= '0' && message[i] <= '9')
                                    {
                                        message[i] = '9' - message[i];
                                    }
                            }
                    }

                sendData(sock, "sendmail");
                sendData(sock, recipient);
                sendData(sock, message);

                recv(sock, buffer, sizeof(buffer), 0);
                cout << buffer << endl;
            } 
        else if (command == "viewmail") 
            {
                sendData(sock, "viewmail");
                char buffer[1024];
                int bytesRecieved = recv(sock, buffer, sizeof(buffer), 0);
                buffer[bytesRecieved] = '\0';

                for(int i = 0 ; i < bytesRecieved ; i++)
                {
                            if(buffer[i] >= 'A' && buffer[i] <= 'Z')
                                {
                                    buffer[i] = 'Z' + 'A' - buffer[i];
                                }
                            else if(buffer[i] >= 'a' && buffer[i] <= 'z')
                                {
                                    buffer[i] = 'z' + 'a' - buffer[i];
                                }
                            else if(buffer[i] >= '0' && buffer[i] <= '9')
                                {
                                    buffer[i] = '9' - buffer[i];
                                }
                }



                cout << buffer << endl;
            } 
        else if (command == "quit") 
            {
                break;
            } 
        else if (command == "logout")
            {
                sendData(sock,"logout");

                char buffer[1024];
                recv(sock,buffer,sizeof(buffer),0);
                buffer[1023] = '\0';
                cout << buffer << endl;
                close(sock);
                exit(0);
            }
        else 
            {
                cout << "Invalid command" << endl;
            }
    }

    close(sock);
    return 0;
}

