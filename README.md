
# 🔐 Sockets Authentication Project (TCP, C++, OpenSSL) SIT716 Task4.1P - Week 1

This project demonstrates secure and insecure authentication over TCP using the **Sockets API** in modern C++. It includes two implementations:

- ✅ **Option 1**: Plaintext username/password authentication
- ✅ **Option 2**: Challenge-response authentication using HMAC-SHA1 (no password sent)

---

## 🧪 How to Compile and Run

> Ensure you have OpenSSL installed on your Ubuntu 22.04 system:

```bash
sudo apt update
sudo apt install libssl-dev
```

### 🧰 Compile (Option 1 - Plaintext Auth)

```bash
g++ server.cpp -o server
g++ client.cpp -o client
```

### 🚀 Run (Option 1)

In **two terminals**:

```bash
# Terminal 1 - Start server
./server

# Terminal 2 - Start client
./client
```

---

### 🧰 Compile (Option 2 - Challenge-Response with HMAC)

```bash
g++ server2.cpp -o server2 -lssl -lcrypto
g++ client2.cpp -o client2 -lssl -lcrypto
```

### 🚀 Run (Option 2)

```bash
# Terminal 1
./server2

# Terminal 2
./client2
```

---

## 🔑 What’s the Difference?

| Feature                     | Option 1 (Plaintext)           | Option 2 (Challenge-Response)           |
|----------------------------|--------------------------------|----------------------------------------|
| Password visibility        | ✅ Visible in Wireshark        | ❌ Not sent over the network           |
| Security level             | ⚠️ Low                         | ✅ Higher (HMAC proof of knowledge)    |
| Protocol style             | Simple exchange                | Requires hashing and secret challenge |
| Recommended for production | ❌ Never                       |  (conceptually aligns with real auth) |

---

## 📸 Screenshots & Explanation

### Option 1 (Insecure)

- 📄 `opt1 - password visible.png`: Password sent as plaintext.
  
  ![1](assets/opt1%20-%20password%20visible.png)

- 📄 `opt1 - authentication succesful.png`: Shows successful login.
  
  ![2](assets/opt1%20-%20authentication%20succesful.png)

- 📄`opt1 - authentication failure.png`: Failed login.
  
  ![3](assets/opt1%20-%20authentication%20failure.png)

- 📄`opt1 cli.png`: Terminal run of client/server.
  
  ![4](assets/opt1%20cli.png)

### Option 2 (Secure)

- 📄 `opt2 - authentication succesful without sending password over network.png`: Challenge-response verification without leaking password.
  
  ![5](assets/opt2%20-%20authentication%20succesful%20without%20sending%20password%20over%20network.png)

- 📄 `opt2 cli.png`: Terminal run with secure HMAC-based flow.
  
  ![6](assets/opt2%20cli.png)

---

## 📚 API Summary

| Function        | From     | Purpose                                               |
|----------------|----------|-------------------------------------------------------|
| `HMAC()`        | OpenSSL  | Calculates a keyed message hash (challenge + secret) |
| `EVP_sha1()`    | OpenSSL  | Selects SHA1 as the hash function for HMAC           |
| `RAND_bytes()`  | OpenSSL  | Generates random bytes for server challenge          |
| `socket()`      | POSIX    | Creates a TCP socket                                  |
| `bind()`        | POSIX    | Binds the server to a port/IP                         |
| `listen()`      | POSIX    | Enables incoming connections                          |
| `accept()`      | POSIX    | Accepts a new connection from a client                |
| `connect()`     | POSIX    | Connects client to server                             |
| `read()`/`send()`| POSIX   | Transmit/receive data over socket                     |
| `close()`       | POSIX    | Closes a file descriptor (socket)                     |
| `htons()`       | C stdlib | Converts port to network byte order                   |
| `inet_pton()`   | POSIX    | Converts text IP to binary format                     |

---

## 🎯 Significance of This Project

This project showcases the importance of **not transmitting credentials** in plaintext. Using **HMAC with OpenSSL**, we simulate how real-world systems like OAuth, SSH, and many APIs authenticate users by proving possession of secrets — without ever exposing them.

By capturing and analyzing packets using **Wireshark**, we validated:

- ✅ Passwords are visible in Option 1
- ✅ Passwords are never transmitted in Option 2
- ✅ HMAC-based challenge-response is verifiable and secure

---

## ✅ Outcome

- Learned Sockets API (`socket`, `bind`, `connect`, etc.)
- Implemented TCP client-server authentication
- Used OpenSSL for cryptographic operations
- Captured and interpreted network packets with Wireshark
- Practiced secure communication patterns
