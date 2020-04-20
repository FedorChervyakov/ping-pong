# Build
FROM gcc:latest AS build

RUN apt-get update && \
    apt-get install -y cmake

ADD ./src /app/src

WORKDIR /app/build

RUN cmake ../src && \
    make

# Run
FROM ubuntu:latest

WORKDIR /app

ADD ./run.sh .

COPY --from=build /app/build/server .
COPY --from=build /app/build/client .

ENV SOCKET_PATH /tmp/socket.sock
ENV LOG_PATH /log/pingpong.log

ENTRYPOINT ["/app/run.sh"]
