#include <iostream>
#include <fstream>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <unistd.h>
#include <arpa/inet.h>
#include <cstring>
#include <sys/socket.h>

using namespace std;

unordered_map<string, string> users; 
unordered_set<string> loggedInUsers; 


void loadUsers() 
{
    ifstream infile("users.txt");
    string username, password;
    while (infile >> username) 
        {
            getline(infile, password);
            if (password.size() > 0 && password[0] == ' ')
                {
                    password.erase(0, 1);
                }
            users[username] = password;
        }
    infile.close();
}



void saveUsers() 
{
    ofstream outfile("users.txt", ios::trunc);
    for (auto& user : users) 
        {
            outfile << user.first << " " << user.second << endl;
        }
    outfile.close();
}




string receiveData(int clientSock) 
{
    char lenBuffer[4]; 
    recv(clientSock, lenBuffer, 4, 0);
    int length = ntohl(*reinterpret_cast<int*>(lenBuffer));
    char buffer[length + 1];
    recv(clientSock, buffer, length, 0); 
    buffer[length] = '\0';
    return string(buffer);
}




void handleClient(int clientSock) 
{
    string command, username, password, recipient, message;

    command = receiveData(clientSock);

    if (command == "register") 
    {
        username = receiveData(clientSock);
      
        password = receiveData(clientSock);

      
        if (users.find(username) != users.end()) 
            {
                send(clientSock, "Username already exists", 23, 0);
            } 
        else 
            {
                users[username] = password;
                saveUsers();
                send(clientSock, "Registration successful", 23, 0);
            }
    }
    
    else if (command == "login") 
        {
            username = receiveData(clientSock);
            password = receiveData(clientSock);

            if (users.find(username) != users.end() && users[username] == password) 
                {
                    loggedInUsers.insert(username);  // Mark user as logged in
                    send(clientSock, "Login successful", 16, 0);
                } 
            else 
                {
                    send(clientSock, "Invalid username or password", 28, 0);
                }
        }

    if (loggedInUsers.find(username) == loggedInUsers.end()) 
        {
            send(clientSock, "You must be logged in to send mail or view mailbox", 52, 0);
            close(clientSock);
            return;
        }

    command = receiveData(clientSock);
    if (command == "sendmail") 
    {
        recipient = receiveData(clientSock);
        message = receiveData(clientSock);

        if (users.find(recipient) == users.end()) 
            {
                send(clientSock, "Recipient not registered", 24, 0);
            } 
        else 
            {
                ofstream mailFile(recipient + "_mailbox.txt", ios::app);
                mailFile << "From: " << username << "\n" << "Message: " << message << endl;
                mailFile.close();
                send(clientSock, "Mail sent successfully", 21, 0);
            }
    } 
    else if (command == "viewmail") 
    {
        ifstream mailFile(username + "_mailbox.txt");
        if (mailFile.is_open()) 
            {
                string line;
                while (getline(mailFile, line)) 
                    {
                        send(clientSock, line.c_str(), line.length(), 0);
                        send(clientSock, "\n", 1, 0);
                    }
                mailFile.close();
            } 
        else 
        {
            send(clientSock, "No mails", 8, 0);
        }
    }

    close(clientSock);
}

int main(void) 
{
    int serverSock, clientSock;
    struct sockaddr_in serverAddr, clientAddr;
    socklen_t clientLen = sizeof(clientAddr);

    loadUsers();


    if ((serverSock = socket(AF_INET, SOCK_STREAM, 0)) == -1) 
        {
            cerr << "Socket creation failed" << endl;
            return 1;
        }

    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(8080);
    serverAddr.sin_addr.s_addr = INADDR_ANY;

    if (bind(serverSock, (struct sockaddr*)&serverAddr, sizeof(serverAddr)) == -1) 
        {
            cerr << "Binding failed" << endl;
            return 1;
        }

 
    if (listen(serverSock, 5) == -1) 
        {
            cerr << "Listen failed" << endl;
            return 1;
        }

    cout << "Server started, waiting for connections..." << endl;

    while (true) 
        {
           
            if ((clientSock = accept(serverSock, (struct sockaddr*)&clientAddr, &clientLen)) == -1) 
                {
                    cerr << "Client connection failed" << endl;
                    continue;
                }

            cout << "Client connected" << endl;

            handleClient(clientSock);
        }

    close(serverSock);
    return 0;
}
