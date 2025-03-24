#include <iostream>    // For std::cin, std::cout, std::cerr
#include <string>      // For std::string
#include <unistd.h>    // For read(), write(), close()
#include <arpa/inet.h> // For inet_pton(), sockaddr_in

// The port number we want to connect to
constexpr int PORT{12345};

// Utility function to convert an IP address string into binary form
// and store it in the sockaddr_in structure for use in socket APIs.
void set_ip_address(sockaddr_in &addr, const std::string &ip_str)
{
    // inet_pton = "presentation to network"
    // Converts the IP address from its text format (e.g., "127.0.0.1")
    // into binary form (network byte order), and stores it in addr.sin_addr

    int result{inet_pton(AF_INET, ip_str.c_str(), &addr.sin_addr)};

    // inet_pton returns:
    //  1 => Success
    //  0 => Not a valid address string
    // -1 => System error (e.g., AF not supported)

    if (result <= 0)
    {
        if (result == 0)
        {
            // IP string was not valid (e.g., "abc.def.ghi.jkl")
            throw std::runtime_error("Invalid IP address format: " + ip_str);
        }
        else
        {
            // Some system-level failure occurred
            throw std::runtime_error("Failed to convert IP address");
        }
    }
}

// Function to create a TCP socket, prepare the server address,
// convert the IP string to binary form, and initiate a connection to the server.
int create_client_socket()
{
    // Step 1: Create a socket
    // AF_INET => IPv4
    // SOCK_STREAM => TCP
    // 0 => Let the OS pick the correct protocol (TCP in this case)
    int sock{socket(AF_INET, SOCK_STREAM, 0)};
    if (sock < 0)
    {
        // socket() returns -1 on failure
        throw std::runtime_error("Socket creation failed");
    }

    // Step 2: Prepare the sockaddr_in structure to hold the server address
    sockaddr_in server_address{};          // Zero-initialize the structure
    server_address.sin_family = AF_INET;   // IPv4 address family
    server_address.sin_port = htons(PORT); // Convert port to network byte order

    // Step 3: Set the server IP address
    // Unlike the server (which binds to INADDR_ANY), the client must specify
    // which IP address it is connecting TO — this is the server's address.
    std::string ip_address{"127.0.0.1"};        // Loopback (localhost)
    set_ip_address(server_address, ip_address); // Converts to binary and stores in sin_addr

    // Step 4: Connect to the server using the socket and address
    // connect() attempts to initiate a TCP connection to the given address.
    // Returns 0 on success, -1 on failure.
    if (connect(sock, reinterpret_cast<sockaddr *>(&server_address), sizeof(server_address)) < 0)
    {
        throw std::runtime_error("Connection failed");
    }

    // Step 5: Return the connected socket file descriptor
    // This socket can now be used with send(), recv(), etc.
    return sock;
}

// Function to read a message from the socket
std::string read_message(int sock)
{
    char buffer[1024]{};
    ssize_t bytes_read{read(sock, buffer, sizeof(buffer))};

    if (bytes_read > 0)
    {
        return std::string(buffer, bytes_read);
    }
    else
    {
        return std::string();
    }
}

// Function to send a message to the server
void send_message(int sock, const std::string &msg)
{
    send(sock, msg.c_str(), msg.length(), 0);
}

// Function to handle the client interaction workflow
void client_interaction(int sock)
{
    // Step 1: Initial greeting exchange
    std::string msg{read_message(sock)};
    std::cout << msg << "\n";

    send_message(sock, "hello");

    // Step 2: Receive prompt for username
    msg = read_message(sock);
    std::cout << msg << "\n";

    std::string username{};
    std::getline(std::cin, username);
    send_message(sock, username);

    // Step 3: Receive prompt for password
    msg = read_message(sock);
    std::cout << msg << "\n";

    std::string password{};
    std::getline(std::cin, password);
    send_message(sock, password);

    // Step 4: Receive authentication result
    msg = read_message(sock);
    std::cout << msg << "\n";
}

// Main function with error handling
int main()
{
    try
    {
        int sock{create_client_socket()};
        client_interaction(sock);
        close(sock); // Always close the socket after use
    }
    catch (const std::exception &e)
    {
        std::cerr << "Client error: " << e.what() << "\n";
    }

    return 0;
}
