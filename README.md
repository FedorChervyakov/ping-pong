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
*headers* and *declarations* are taken straight from manpage corresponding to each syscall.

### Sockets
#### Accepting connections
To accept connections, the following steps are performed:

1.  A socket is created with socket(2).
2.  The socket is bound to a local address using bind(2), so that other sockets may be connect(2)ed to it.
3.  A willingness to accept incoming connections and a queue limit for incoming connections are specified with listen().
4.  Connections are accepted with accept(2).

### Other notes
List open Unix sockets
`lsof -U`

### Syscalls API
#### open()
__headers__
```c
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
```
__declarations__
```c
int open(const char *pathname, int flags);
int open(const char *pathname, int flags, mode_t mode);

int creat(const char *pathname, mode_t mode);

int openat(int dirfd, const char *pathname, int flags);
int openat(int dirfd, const char *pathname, int flags, mode_t mode);
```

#### socket()
__headers__
```c
#include <sys/types.h>
#include <sys/socket.h>
```
__declaration__
```c
int socket(int domain, int type, int protocol);
```

#### listen()
__headers__
```c
#include <sys/types.h>
#include <sys/socket.h>
```
__declaration__
```c
int listen(int sockfd, int backlog);
```
