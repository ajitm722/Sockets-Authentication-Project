#include <iostream>     // For std::cout, std::cerr, std::string, etc.
#include <string>       // For std::string
#include <netinet/in.h> // For sockaddr_in, htons, bind(), listen(), etc.
#include <unistd.h>     // For read(), write(), close()

// Port the server will listen on
constexpr int PORT{12345};

// Function to create, bind, and set up the server socket
int create_server_socket()
{
    // Create a socket with IPv4 (AF_INET), TCP (SOCK_STREAM), and default protocol (0)
    int sockfd{socket(AF_INET, SOCK_STREAM, 0)};
    if (sockfd < 0)
    {
        throw std::runtime_error("Socket creation failed");
    }

    // Define the server address (IP: ANY, Port: PORT)
    sockaddr_in address{};
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY; // Listen on all interfaces
    address.sin_port = htons(PORT);       // Convert port to network byte order

    // Bind socket to the specified IP and port
    if (bind(sockfd, reinterpret_cast<sockaddr *>(&address), sizeof(address)) < 0)
    {
        throw std::runtime_error("Bind failed");
    }

    // Listen for incoming connections (max 1 pending connection)
    if (listen(sockfd, 1) < 0)
    {
        throw std::runtime_error("Listen failed");
    }

    return sockfd; // Return the server socket file descriptor
}

// Read a message from a connected socket
std::string read_message(int sock)
{
    char buffer[1024]{};
    ssize_t bytes_read{read(sock, buffer, sizeof(buffer))};

    if (bytes_read > 0)
    {
        return std::string(buffer, bytes_read); // Return message as string
    }
    else
    {
        return std::string();
    }
}

// Send a message through the connected socket
void send_message(int sock, const std::string &msg)
{
    send(sock, msg.c_str(), msg.length(), 0);
}

// Handle client-server interaction
void handle_client(int client_sock)
{
    // Step 1: Initial greeting
    send_message(client_sock, "Hello. Send your greeting.");

    std::string hello{read_message(client_sock)};
    std::cout << "Client says: " << hello << "\n";

    // Step 2: Ask for username
    send_message(client_sock, "Enter username:");
    std::string username{read_message(client_sock)};

    // Step 3: Ask for password
    send_message(client_sock, "Enter password:");
    std::string password{read_message(client_sock)};

    // Step 4: Verify credentials
    std::string response{};

    if (username == "admin" && password == "pass123")
    {
        response = "Authentication successful.\n secret_data_from_server...";
        send_message(client_sock, response);
    }
    else
    {
        response = "Authentication failed.";
        send_message(client_sock, response);
    }

    // Step 6: Close client connection
    close(client_sock);
}

int main()
{
    try
    {
        // Step 1: Create and set up server socket
        int server_sock{create_server_socket()};
        std::cout << "Server listening on port " << PORT << "...\n";

        // Step 2: Accept one client connection
        sockaddr_in client_addr{};
        socklen_t addr_len{sizeof(client_addr)};
        int client_sock{accept(server_sock, reinterpret_cast<sockaddr *>(&client_addr), &addr_len)};
        if (client_sock < 0)
        {
            throw std::runtime_error("Accept failed");
        }

        // Step 3: Handle client interaction
        handle_client(client_sock);

        // Step 4: Close the main server socket
        close(server_sock);
    }
    catch (const std::exception &e)
    {
        std::cerr << "Server error: " << e.what() << "\n";
    }

    return 0;
}
