# Sockets - connection between Client-Server

- Source :
* [Programmation réseau via socket](https://www.codequoi.com/programmation-reseau-via-socket-en-c/)
* [Video Sockets d'1h](https://www.youtube.com/watch?v=oYBgV474Udc)
* [Socket Programming GeeksForGeeks](https://www.geeksforgeeks.org/socket-programming-cc/)
* [Inner workings of the Webserver](https://hackmd.io/@laian/SJZHcOsmT#Sockets-and-Useful-Network-Functions)
* [Handling sockets with C++](https://ncona.com/2019/04/building-a-simple-server-with-cpp/)
* [Video d'1h - Network Programming - C++ Sockets](https://www.youtube.com/watch?v=gntyAFoZp-E)

The connection between clients and servers are facilitated by sockets, which are the *communication link between two processes on a network*.

A ***socket is a file descriptor created using this function*** :

```cpp
int	socket(int, domain, int type, int protocol);
```
- `domain` = refers to the communication domain. [Domain list here](https://linux.die.net/man/2/socket)
- `type` = type of socket.
- `protocol` = refers to a particular protocol to be used with sockets, usually is **0**.

Once we have a socket descriptor, we need to ***bind it to a port on the computer*** with:
```cpp
int	bind(int sockfd, const struct sockaddr *addr, socklen_t addrlen);
```

#### Types of sockets

- **Stream Sockets** -> `SOCK_STREAM`
-> are reliable two-way connected communication streams which uses TCP. These sockets are usually used by HTTP when speed is not the priority, but data quality is.

- **Datagram Sockets** -> `SOCK_DGRAM`
-> are connectionless sockets that use the UDP (User Datagram Protocol). So when data is sent, it may not arrive. These sockets are commonly used by games/video/audio where speed is priority.

