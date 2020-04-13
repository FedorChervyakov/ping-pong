# Ping-Pong
Basic ping-pong on sockets. Written to practice various syscalls.

## Requirements
Listed below are requirements for the code.

### Build system
GCC and CMake

### Syscalls
- `open()` with various flags
- `close()`
- `socket()`
- `listen()`
- `accept()`
- `ioctl()`
- `write()`
- `read()`

## Notes

### Sockets
#### Accepting connections
To accept connections, the following steps are performed:

1.  A socket is created with socket(2).
2.  The socket is bound to a local address using bind(2), so that other
sockets may be connect(2)ed to it.
3.  A willingness to accept incoming connections and a queue limit for 
incoming connections are specified with listen().
4.  Connections are accepted with accept(2).

### Other notes
List open Unix sockets
`lsof -U`
