#include <iostream>       // For std::cout, std::cerr
#include <string>         // For std::string
#include <unistd.h>       // For POSIX system calls: read(), write(), close()
#include <arpa/inet.h>    // For sockaddr_in, inet_pton, htons
#include <openssl/hmac.h> // For HMAC() using SHA1

// === Constants ===
constexpr int PORT{12345};                  // Server port to connect to
const std::string SHARED_SECRET{"pass123"}; // Shared key known to both client and server

// === Function: Compute HMAC ===
// Computes a HMAC-SHA1 of the input data using the shared key.
// Used to respond to server's challenge in challenge-response auth.
//
// Note: OpenSSL's HMAC() returns a static buffer. We must copy it immediately.
// We convert the raw binary result to std::string (which may contain nulls).
std::string compute_hmac(const std::string &data, const std::string &key)
{
    unsigned int len{20}; // SHA1 = 160-bit = 20 bytes

    // Get raw pointer to 20-byte HMAC digest (computed by OpenSSL)
    unsigned char *raw_result = HMAC(
        EVP_sha1(),                                                           // Hash function (SHA1)
        key.c_str(), static_cast<int>(key.length()),                          // Key buffer + length
        reinterpret_cast<const unsigned char *>(data.c_str()), data.length(), // Message
        nullptr, nullptr                                                      // Not using output len pointer
    );

    if (!raw_result)
    {
        throw std::runtime_error("HMAC computation failed");
    }

    // reinterpret_cast is needed to treat raw bytes as char array
    // Safe here because we copy `len` bytes explicitly into the string
    return std::string(reinterpret_cast<char *>(raw_result), len);
}

// === Function: Create and connect TCP socket to server ===
int create_client_socket()
{
    // Step 1: Create TCP socket (IPv4)
    int sock{socket(AF_INET, SOCK_STREAM, 0)};
    if (sock < 0)
    {
        throw std::runtime_error("Socket creation failed");
    }

    // Step 2: Configure server address structure
    sockaddr_in server_address{};
    server_address.sin_family = AF_INET;   // IPv4
    server_address.sin_port = htons(PORT); // Convert port to network byte order

    // Convert IP string ("127.0.0.1") to binary form for socket API
    // inet_pton returns 1 on success, 0 for invalid format, -1 on error
    std::string ip{"127.0.0.1"};
    int result = inet_pton(AF_INET, ip.c_str(), &server_address.sin_addr);
    if (result <= 0)
    {
        throw std::runtime_error("Invalid or unsupported IP address");
    }

    // Step 3: Connect to the server
    if (connect(sock, reinterpret_cast<sockaddr *>(&server_address), sizeof(server_address)) < 0)
    {
        throw std::runtime_error("Connection failed");
    }

    return sock;
}

// === Function: Read data from socket ===
std::string read_message(const int sock)
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

// === Function: Send string message to socket ===
void send_message(const int sock, const std::string &msg)
{
    send(sock, msg.c_str(), msg.length(), 0); // Standard POSIX send()
}

// === Function: Perform challenge-response protocol with server ===
void client_interaction(const int sock)
{
    // Step 1: Send initial hello to initiate conversation
    send_message(sock, "hello");

    // Step 2: Receive challenge string from server
    std::string challenge{read_message(sock)};
    std::cout << "Received challenge: " << challenge << "\n";

    // Step 3: Compute HMAC of challenge using shared secret
    std::string digest{compute_hmac(challenge, SHARED_SECRET)};

    // Step 4: Send computed digest back to server
    send_message(sock, digest);

    // Step 5: Receive authentication result (success or failure)
    std::string response{read_message(sock)};
    std::cout << "Server: " << response << "\n";
}

// === Main Entry Point ===
int main()
{
    try
    {
        // Connect to the server
        int sock{create_client_socket()};

        // Run the client-side interaction
        client_interaction(sock);

        // Cleanly close the socket
        close(sock);
    }
    catch (const std::exception &e)
    {
        std::cerr << "Client error: " << e.what() << "\n";
    }

    return 0;
}
