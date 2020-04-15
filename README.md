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
```
# From build dir:
./pp-server some.sock
# Then, in another terminal:
./pp-client some.sock
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

