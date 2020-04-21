# Build
FROM gcc:latest AS build

RUN apt-get update && \
    apt-get install -y cmake

ADD ./src /app/src
ADD ./log.c /app/log.c
ADD ./CMakeLists.txt /app

WORKDIR /app/build

RUN cmake .. && cmake --build .

# Run
FROM ubuntu:latest

WORKDIR /app

ADD ./run.sh .

COPY --from=build /app/build/bin/server .
COPY --from=build /app/build/bin/client .
COPY --from=build /app/build/lib/ /usr/lib

ENV SOCKET_PATH /tmp/socket.sock
ENV LOG_PATH /log/pingpong.log

ENTRYPOINT ["/app/run.sh"]
