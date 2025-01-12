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

#define PORT 8080
#define BUFFER_SIZE 1024

using namespace std;

mutex fileMutex;  // Mutex to prevent file access conflicts

// Struct to represent a user
struct User 
{
    string password;
};

// Map to store registered users
map<string, User> users;

// Function to load users from the "users.txt" file
void loadUsers() 
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

// Function to save a user to "users.txt"
void saveUser(const string& username, const string& password) 
{
    lock_guard<mutex> lock(fileMutex);
    ofstream outFile("users.txt", ios::app);
    outFile << username << ":" << password << endl;
    outFile.close();
}

// Function to store mail in the recipient's file
void storeMail(const string& recipient, const string& mailContent) 
{
    lock_guard<mutex> lock(fileMutex);
    ofstream outFile(recipient + "_mail.txt", ios::app);
    outFile << mailContent << endl;
    outFile.close();
}

// Handle client registration
string handleRegistration(const string& username, const string& password) 
{
    if (users.find(username) != users.end()) 
    {
        return "Error: Username already exists.\n";
    }

    users[username] = {password};
    saveUser(username, password);
    return "Registration successful.\n";
}

// Handle client login
bool handleLogin(const string& username, const string& password) 
{
    auto it = users.find(username);
    return it != users.end() && it->second.password == password;
}

// Handle sending mail
string handleSendMail(const string& sender, const string& recipient, const string& mailContent) 
{
    auto it = users.find(recipient);
    if (it == users.end()) 
    {
        return "Error: Recipient does not exist.\n";
    }

    string fullMail = "From: " + sender + "\nMessage: " + mailContent + "\n";
    storeMail(recipient, fullMail);
    return "Mail sent successfully.\n";
}

// Handle viewing inbox
string handleViewInbox(const string& username) 
{
    lock_guard<mutex> lock(fileMutex);
    ifstream inFile(username + "_mail.txt");
    if (!inFile.is_open()) 
    {
        return "No mails found.\n";
    }

    stringstream inbox;
    inbox << "Inbox:\n";
    string line;
    while (getline(inFile, line)) 
    {
        inbox << line << endl;
    }
    inFile.close();
    return inbox.str();
}

// Function to handle a single client session
void handleClient(int clientSocket) 
{
    char buffer[BUFFER_SIZE] = {0};
    string username;
    bool isLoggedIn = false;

    while (true) 
    {
        memset(buffer, 0, BUFFER_SIZE);
        read(clientSocket, buffer, BUFFER_SIZE);

        string request(buffer);
        stringstream ss(request);
        string command, password, recipient, mailContent;
        getline(ss, command, ':');

        string response;

        if (command == "register") 
        {
            getline(ss, username, ':');
            getline(ss, password, ':');
            response = handleRegistration(username, password);

        } 
        else if (command == "login") 
        {
            getline(ss, username, ':');
            getline(ss, password, ':');
            if (handleLogin(username, password)) 
            {
                isLoggedIn = true;
                response = "Login successful.\n";
            } 
            else 
            {
                response = "Error: Invalid username or password.\n";
            }

        } 
        else if (command == "send_mail" && isLoggedIn) 
        {
            getline(ss, recipient, ':');
            getline(ss, mailContent);
            response = handleSendMail(username, recipient, mailContent);

        } 
        else if (command == "view_inbox" && isLoggedIn) 
        {
            response = handleViewInbox(username);

        } 
        else if (command == "logout" && isLoggedIn) 
        {
            response = "Logout successful.\n";
            isLoggedIn = false;

        } 
        else if (command == "exit") 
        {
            response = "Goodbye.\n";
            send(clientSocket, response.c_str(), response.length(), 0);
            break;

        }
        else 
        {
            response = "Error: Invalid command or not logged in.\n";
        }

        send(clientSocket, response.c_str(), response.length(), 0);
    }

    close(clientSocket);
}

int main() {
    int serverFd, newSocket;
    struct sockaddr_in address;
    int opt = 1;
    int addrlen = sizeof(address);

    loadUsers();

    if ((serverFd = socket(AF_INET, SOCK_STREAM, 0)) == 0) 
    {
        perror("Socket failed");
        exit(EXIT_FAILURE);
    }

    if (setsockopt(serverFd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt))) 
    {
        perror("setsockopt");
        exit(EXIT_FAILURE);
    }

    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT);

    if (bind(serverFd, (struct sockaddr*)&address, sizeof(address)) < 0) 
    {
        perror("Bind failed");
        exit(EXIT_FAILURE);
    }

    if (listen(serverFd, 5) < 0) 
    {
        perror("Listen");
        exit(EXIT_FAILURE);
    }

    cout << "Server is running on port " << PORT << endl;

    while (true) 
    {
        if ((newSocket = accept(serverFd, (struct sockaddr*)&address, (socklen_t*)&addrlen)) < 0) 
        {
            perror("Accept");
            exit(EXIT_FAILURE);
        }

        thread clientThread(handleClient, newSocket);
        clientThread.detach();
    }

    return 0;
}
