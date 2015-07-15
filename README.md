# El Gamal Encrypted Chat

Server / Client Unix chat implementation based on "El Gamal" Encryption Scheme.
Extension of http://www.theinsanetechie.in/2014/01/a-simple-chat-program-in-c-tcp.html.

# How to compile

Compile Server / Client separately:
```
gcc -pthread -o server server_chat.c
gcc -pthread .o client client_chat.c
```

Then start server and client in two different shells:
```
./server
./client
```
# Instructions

You can modify the numbers exchanged during the server/client handshaking inside the code (#DEFINE P ecc.).
Be aware that numbers are stored in "long" variables, so try to avoid using very long digits.

Feel free to clone / fork / modify it ;)

