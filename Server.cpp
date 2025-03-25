#include <iostream>
#include <fstream>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <unistd.h>
#include <arpa/inet.h>
#include <cstring>
#include <cstdbool>
#include <cstdlib>
#include <aliases.h>
#include <cstdio>
#include <fstream>
#include <fcntl.h>
#include <sys/socket.h>
using namespace std;


unordered_set<string> loggedInUsers;
unordered_map<string, string> users;  



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
                }
        }
        return output;
        
    }


void loadUsers() 
{
    ifstream infile("users.txt");
    if(!infile)
        {
            cout << "Server become hacked... wait.. " << endl;
            ofstream newFile("users.txt");
            newFile.close();
            return;
        }
    string username, password;
    while (infile >> username) 
        {
            getline(infile, password);
            if (password.size() > 0 && password[0] == ' ')
                {
                    password.erase(0, 1);
                }
            
            for(int i = 0 ; i < password.size() ; i++)
                {
                    if(isalpha(password[i]))
                        {
                            if(password[i] >= 'A' && password[i] <= 'Z')
                            {
                                password[i] = 'Z' + 'A' - password[i];
                            }
                            else if(password[i] >= 'a' && password[i] <= 'z')
                                {
                                    password[i] = 'z' + 'a' - password[i];
                                }
 
                        }
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
            string text;
            for(int i = 0 ; i < user.second.size() ; i++)
                {
                    text.push_back(user.second[i]);
                }
            
            for(int i = 0 ; i < text.size() ; i++)
                {
                    if(text[i] >= 'A' && text[i] <= 'Z')
                        {
                            text[i] = 'A' + 'Z' - text[i];
                        }
                        else if(text[i] >= 'a' && text[i] <= 'z')
                        {
                            text[i] = 'z' + 'a' - text[i];
                        }
                }
            

            outfile << user.first << " " << text << endl;
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





bool searchInFile(const char* filename, const string& searchTerm) {
    int fd = open(filename, O_RDONLY);  
    if (fd == -1) {
        cout << "Error opening file!" << endl;
        return false;
    }

    char buffer[1024];  
    int bytesRead;
    string fileContent;

    while ((bytesRead = read(fd, buffer, sizeof(buffer))) > 0) {
        fileContent.append(buffer, bytesRead);

        size_t pos = fileContent.find(searchTerm);
        if (pos != string::npos) {
            close(fd);
            return true; 
        }

        if (bytesRead == sizeof(buffer)) {
            fileContent.erase(0, fileContent.size() - searchTerm.size());
        }
    }

    close(fd);
    return false;
}




void handleClient(int clientSock) 
{
    string command, username, password, recipient, message,newPass;

    command = receiveData(clientSock);

    if (command == "register") 
    {
        username = receiveData(clientSock);
      
        password = receiveData(clientSock);

      
        if (users.find(username) != users.end()) 
            {
                string msge = "This username already exis";
                send(clientSock,msge.c_str(), msge.size(), 0);
            } 
        else 
            {
                users[username] = password;
                saveUsers();
                string msge = "Your registration succesful";
                send(clientSock, msge.c_str(), msge.size(), 0);
            }
    }
    
    else if (command == "login") 
        {
            username = receiveData(clientSock);
            password = receiveData(clientSock);

            if (users.find(username) != users.end() && users[username] == password) 
                {
                    loggedInUsers.insert(username);
                    string msge = "Your login succesful";
                    send(clientSock, msge.c_str(), msge.size(), 0);
                } 
            else 
                {
                    string msge = "Invalid username or password won't match";
                    send(clientSock, msge.c_str(), msge.size(), 0);
                }
        }

    if (loggedInUsers.find(username) == loggedInUsers.end()) 
        {
            string msge = "You have to loggedin first";
            send(clientSock, msge.c_str(), msge.size(), 0);
            close(clientSock);
            return;
        }

    command = receiveData(clientSock);
    if (command == "sendmail") 
    {
        
        if (loggedInUsers.find(username) == loggedInUsers.end()) 
        {
            string msge = "You have to loggedin first";
            send(clientSock, msge.c_str(), msge.size(), 0);
            close(clientSock);
            return;
        }

        recipient = receiveData(clientSock);
        message = receiveData(clientSock);

        if (users.find(recipient) == users.end()) 
            {
                string msge = "This recipient not registered to the server";
                send(clientSock, msge.c_str(), msge.size(), 0);
            } 
        else 
            {
                ofstream mailFile(recipient + "_mailbox.txt", ios::app);
                mailFile << "From: " << username << "\n" << "Message: " << message << endl;
                mailFile.close();
                string msge = "Mail sended";
                send(clientSock, msge.c_str(), msge.size(), 0);
            }
    } 
    else if (command == "changepass")
        {
            
            if (loggedInUsers.find(username) == loggedInUsers.end()) 
        {
            string msge = "You have to loggedin first";
            send(clientSock, msge.c_str(), msge.size(), 0);
            close(clientSock);
            return;
        }
            username = receiveData(clientSock);
            password = receiveData(clientSock);
            newPass = receiveData(clientSock);

            if(loggedInUsers.find(username) == loggedInUsers.end())
                {
                    string msge = "Please login first then try to change password";
                    send(clientSock,msge.c_str(),msge.size(),0);
                }
            else if(users.find(username) != users.end() && users[username] == password)
                {
                    users[username] = newPass;
                    saveUsers();
                    string msge = "Your password changed successfully";
                    send(clientSock,msge.c_str(),msge.size(),0);
                }
            else
                {
                    string msge = "Invalid old password";
                    send(clientSock,msge.c_str(),msge.size(),0);
                }
        }
    else if (command == "viewmail") 
    {
        if (loggedInUsers.find(username) == loggedInUsers.end()) 
        {
            string msge = "You have to loggedin first";
            send(clientSock, msge.c_str(), msge.size(), 0);
            close(clientSock);
            return;
        }
        
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
            string msge = "There is no mail";
            send(clientSock, msge.c_str(), msge.size(), 0);
        }
    }
    else if (command == "logout")
        {
            if (loggedInUsers.find(username) == loggedInUsers.end()) 
            {
                string msge = "You have to loggedin first";
                send(clientSock, msge.c_str(), msge.size(), 0);
                close(clientSock);
                return;
            }
            else
                {
                    string msge = "You aren't logged in";
                    send(clientSock,msge.c_str(),msge.size(),0);
                }
            close(clientSock);
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
            cout << "Socket creation failed" << endl;
            return 1;
        }

    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(8080);
    serverAddr.sin_addr.s_addr = INADDR_ANY;

    if (bind(serverSock, (struct sockaddr*)&serverAddr, sizeof(serverAddr)) == -1) 
        {
            cout << "Binding failed" << endl;
            return 1;
        }

 
    if (listen(serverSock, 5) == -1) 
        {
            cout << "Listen failed" << endl;
            return 1;
        }

    cout << "Server started, waiting for connections..." << endl;

    while (true) 
        {
           
            if ((clientSock = accept(serverSock, (struct sockaddr*)&clientAddr, &clientLen)) == -1) 
                {
                    cout << "Client connection failed" << endl;
                    continue;
                }

            cout << "Client connected" << endl;

            handleClient(clientSock);
        }
    

    close(serverSock);
    return 0;
}
