#include <iostream>
#include <fstream>
#include <sstream>
#include <cstring>
#include <map>
#include <thread>
#include <mutex>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
using namespace std;
#define PORT 8080
#define BUFFER_SIZE 1024



struct User 
{
    string password;
};
map<string, User> users;
mutex fileMutex;

void loadUsers(void);
void saveUser(string username,string password);
void storeMail(string& recipient,string& mailMassage);
string checkRegistration(string username,string password);
bool handleLogin(string username,string password);
string SendMail(string sender,string recipent,string mailMassage);
string Vieew(string username);
void handleClient(int clientSocket);

int main(void) 
{
    int serverFd, newSocket;
    struct sockaddr_in address;
    int opt = 1;
    int addrlen = sizeof(address);

    loadUsers();

    if ((serverFd = socket(AF_INET, SOCK_STREAM, 0)) == 0) 
    {
        cout << "Yout socket failed" << endl;
        exit(EXIT_FAILURE);
    }

    if (setsockopt(serverFd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt))) 
    {
        cout << "setsockopt" << endl;
        exit(EXIT_FAILURE);
    }

    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT);

    if (bind(serverFd, (struct sockaddr*)&address, sizeof(address)) < 0) 
    {
        cout << "Bind Failed" << endl;
        exit(EXIT_FAILURE);
    }

    if (listen(serverFd, 5) < 0) 
    {
        cout << "Listen";
        exit(EXIT_FAILURE);
    }

    cout << "Server is running on port " << PORT << endl;

    bool flag = true;
    for( ; flag ; )
    {
        if ((newSocket = accept(serverFd, (struct sockaddr*)&address, (socklen_t*)&addrlen)) < 0) 
        {
            cout << "Accept";
            exit(EXIT_FAILURE);
        }

        thread clientThread(handleClient, newSocket);
        clientThread.detach();   
    }

    return 0;
}


void loadUsers(void) 
{
    ifstream inFile("users.txt");
    string line;
    while (getline(inFile, line)) 
    {
        size_t separator = line.find(':');
        if (separator != string::npos) 
        {
            string username = line.substr(0, separator);
            string password = line.substr(separator + 1);
            users[username] = {password};
        }
    }
    inFile.close();
}

void saveUser(string username,string password) 
{
    lock_guard<mutex> lock(fileMutex);
    ofstream outFile("users.txt", ios::app);
    outFile << username << ":" << password << endl;
    outFile.close();
}


void storeMail(string& recipient,string& mailMassage) 
{
    lock_guard<mutex> lock(fileMutex);
    ofstream outFile(recipient + "_mail.txt", ios::app);
    outFile << mailMassage << endl;
    outFile.close();
}

string checkRegistration(string username,string password)
{
    if (users.find(username) != users.end()) 
    {
        return "This username is already used please try different\n";
    }

    users[username] = {password};
    saveUser(username, password);
    return "Your registration completed\n";
}

bool handleLogin(string username,string password) 
{
    auto it = users.find(username);
    if(it != users.end() && it -> second.password == password)
    {
        return true;
    }
    return false;
}



string SendMail(string sender,string recipient,string mailMassage) 
{
    auto it = users.find(recipient);
    if (it == users.end()) 
    {
        return "This user isn't register in my seerver\n";
    }

    string fullMail = "From: " + sender + "\nMessage: " + mailMassage + "\n";
    storeMail(recipient, fullMail);
    return "Yourr mail send successfully\n";
}


string Vieew(string username) 
{
    lock_guard<mutex> lock(fileMutex);
    ifstream inFile(username + "_mail.txt");
    if (!inFile.is_open()) 
    {
        return "You don't have any mail\n";
    }

    stringstream inbox;
    inbox << "your Inbox:\n";
    string line;
    while (getline(inFile, line)) 
    {
        inbox << line << endl;
    }
    inFile.close();
    return inbox.str();
}


void handleClient(int clientSocket)
{
    char buffer[BUFFER_SIZE] = {0};
    string username;
    bool login = false;
    while(1)
    {
        memset(buffer,0,BUFFER_SIZE);
        read(clientSocket,buffer,BUFFER_SIZE);

        string request(buffer);
        stringstream ss(request);
        string commandd,password,recipient,mailMassage;
        getline(ss,commandd,':');
        string response;

        if(commandd == "register")
        {
            getline(ss,username,':');
            getline(ss,password,':');
            response = checkRegistration(username,password);
        }
        else if (commandd == "login")
        {
            getline(ss,username,':');
            getline(ss,password,':');
            if(handleLogin(username,password))
            {
                login = true;
                response = "login succesful\n";
            }
            else
            {
                response = "Wrong password or username\n";
            }
        }

        else if(commandd == "send_mail")
        {
            cout << "Enter the recipient ";
            cin >> recipient;
            cin.ignore();
            cout << "Enter the massage ";
            getline(cin,mailMassage);
            request += ":" + recipient + ":" + mailMassage;
        }
        else if(commandd == "Mailbox")
        {
            if(login)
            {
                response = Vieew(username);
            }
            else
            {
                response = "Please login firstly\n" ;
            }
        }
        else if(commandd == "logout")
        {
            if(login)
            {
                response = "Logout Succesfull\n";
                login = false;
            }
            else
            {
                response = "You are not logged in how can you logout\n";
            }
        }
        else if(commandd == "exit")
        {
            response = "Al bida tata bye bye \n";
            send(clientSocket,response.c_str(),response.length(),0);
            break;
        }
        else
        {
            response = "Your command isn't valid\n";
        }
        send(clientSocket,response.c_str(),response.length(),0);
    }
    close(clientSocket);
}
