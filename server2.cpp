#include <iostream>       // For std::cout, std::cerr, std::string, etc.
#include <string>         // For std::string
#include <netinet/in.h>   // For sockaddr_in, htons, INADDR_ANY, etc.
#include <unistd.h>       // For POSIX socket functions: read(), write(), close()
#include <openssl/hmac.h> // For HMAC (Hash-based Message Authentication Code)
#include <openssl/rand.h> // For RAND_bytes() – cryptographically secure RNG

// === CONSTANTS ===

// TCP port number that the server will bind to
constexpr int PORT{12345};

// A secret key shared between client and server
// It's never sent over the network — only used for hashing
const std::string SHARED_SECRET{"pass123"};

// === FUNCTION: Generate a Random Challenge String ===
// This function creates a cryptographically secure random byte string (challenge)
// which will be used for the HMAC challenge-response step.
std::string generate_challenge(size_t length = 16)
{
    unsigned char buffer[64]{}; // Buffer with maximum safe size
    if (!RAND_bytes(buffer, static_cast<int>(length)))
    {
        throw std::runtime_error("Failed to generate random challenge");
    }
    // reinterpret_cast is used to treat raw bytes as char* for std::string construction
    // This is safe because we're specifying the exact length and OpenSSL guarantees buffer is filled
    return std::string(reinterpret_cast<char *>(buffer), length);
}

// === FUNCTION: Compute HMAC using SHA1 ===
// Parameters:
// - data: the challenge string to hash
// - key: the shared secret (known to both client and server)
//
// Returns:
// - A binary string (raw bytes) that represents the HMAC result
std::string compute_hmac(const std::string &data, const std::string &key)
{
    // HMAC() is an OpenSSL-provided function that calculates a keyed hash
    // using a specified algorithm (here, SHA1)

    // SHA1 produces a 160-bit (20 byte) result
    unsigned int len{20};

    // This is a pointer to a static internal buffer managed by OpenSSL.
    // We do NOT free this memory, and we SHOULD copy the contents out immediately.
    unsigned char *raw_result = HMAC(
        EVP_sha1(),                                                           // Use SHA1 as the hash function
        key.c_str(), static_cast<int>(key.length()),                          // The key (shared secret)
        reinterpret_cast<const unsigned char *>(data.c_str()), data.length(), // Message
        nullptr, nullptr                                                      // Output buffer (NULL = use internal)
    );

    if (!raw_result)
    {
        throw std::runtime_error("HMAC computation failed");
    }

    // Explanation:
    // Even though HMAC returns a raw pointer, we do NOT use smart pointers here because:
    // - The memory is not heap-allocated (no malloc/free involved)
    // - OpenSSL manages its own internal buffers
    // - Wrapping in smart pointers would give a false sense of ownership

    // Convert the raw binary result into a C++ string
    // Note: This string may contain null bytes (\0), which is safe as we specify length
    // reinterpret_cast is used here to convert raw bytes into a char* for constructing a std::string
    // The resulting std::string may contain null bytes, but that's okay since we specify the length
    return std::string(reinterpret_cast<char *>(raw_result), len);
}

// === FUNCTION: Create and Prepare the Server Socket ===
int create_server_socket()
{
    // Create a socket: AF_INET = IPv4, SOCK_STREAM = TCP
    int sockfd{socket(AF_INET, SOCK_STREAM, 0)};
    if (sockfd < 0)
    {
        throw std::runtime_error("Socket creation failed");
    }

    // Prepare address structure
    sockaddr_in address{};
    address.sin_family = AF_INET;         // IPv4
    address.sin_addr.s_addr = INADDR_ANY; // Accept connections on any interface
    address.sin_port = htons(PORT);       // Convert port number to network byte order (big endian)

    // Bind the socket to the port/IP
    if (bind(sockfd, reinterpret_cast<sockaddr *>(&address), sizeof(address)) < 0)
    {
        throw std::runtime_error("Bind failed");
    }

    // Start listening for incoming TCP connections
    if (listen(sockfd, 1) < 0)
    {
        throw std::runtime_error("Listen failed");
    }

    return sockfd;
}

// === FUNCTION: Read data from socket ===
std::string read_message(const int sock)
{
    char buffer[1024]{};                                    // Temporary buffer for incoming message
    ssize_t bytes_read{read(sock, buffer, sizeof(buffer))}; // POSIX read()

    // Use explicit if-else for clarity instead of ternary
    if (bytes_read > 0)
    {
        return std::string(buffer, bytes_read);
    }
    else
    {
        return std::string();
    }
}

// === FUNCTION: Send message to socket ===
void send_message(const int sock, const std::string &msg)
{
    // Write the entire message over the TCP connection
    send(sock, msg.c_str(), msg.length(), 0);
}

// === FUNCTION: Handle One Client Session ===
void handle_client(const int client_sock)
{
    // Step 1: Expect "hello" from client
    std::string hello{read_message(client_sock)};
    std::cout << "Client: " << hello << "\n";

    // Step 2: Generate a random challenge and send it to the client
    std::string challenge{generate_challenge()};
    send_message(client_sock, challenge);

    // Step 3: Receive client’s HMAC digest
    std::string client_digest{read_message(client_sock)};

    // Step 4: Compute our own digest using the same challenge + secret
    std::string expected_digest{compute_hmac(challenge, SHARED_SECRET)};

    // Step 5: Compare the two HMAC results
    std::string response{};
    if (client_digest == expected_digest)
    {
        response = "Authentication successful. Welcome!";
    }
    else
    {
        response = "Authentication failed.";
    }

    // Step 6: Send result back to client
    send_message(client_sock, response);

    // Step 7: Close client connection
    close(client_sock);
}

// === MAIN ===
int main()
{
    try
    {
        // Create server socket and begin listening
        int server_sock = create_server_socket();
        std::cout << "Server listening on port " << PORT << "...\n";

        // Wait for a single client connection
        sockaddr_in client_addr{};
        socklen_t addr_len{sizeof(client_addr)};
        int client_sock = accept(server_sock, reinterpret_cast<sockaddr *>(&client_addr), &addr_len);

        if (client_sock < 0)
        {
            throw std::runtime_error("Accept failed");
        }

        // Handle the connected client session
        handle_client(client_sock);

        // Close the server socket after handling the client
        close(server_sock);
    }
    catch (const std::exception &e)
    {
        std::cerr << "Server error: " << e.what() << "\n";
    }

    return 0;
}
