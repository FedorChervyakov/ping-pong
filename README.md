[![Build Status](https://travis-ci.org/FedorChervyakov/ping-pong.svg?branch=dev)](https://travis-ci.org/FedorChervyakov/ping-pong)

# Ping-Pong
Basic ping-pong on sockets. Written to practice various syscalls.

## Install
CMake>=3.10 and GCC are required.
```
git clone git@github.com:FedorChervyakov/ping-pong.git
cd ping-pong
mkdir build
cd build
cmake ../src
make
```

## Usage
### Unix sockets
```
./server -u some.sock
# Then, in another terminal:
./client -u some.sock
```
### TCP sockets
`-4` is used to force IPv4. Use `-6` to force IPv6.
```
./server -4 -L '*:12345'
# Then, in another terminal:
./client -4 -C 'localhost:12345'
```

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

