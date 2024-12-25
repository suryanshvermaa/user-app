FROM drogonframework/drogon AS builder

WORKDIR /app

COPY . .
RUN mkdir -p build

WORKDIR /app/jwt-cpp
RUN mkdir -p build
WORKDIR /app/jwt-cpp/build
RUN cmake ..
RUN make
RUN make install

WORKDIR /app/libbcrypt
RUN mkdir -p build
WORKDIR /app/libbcrypt/build
RUN cmake ..
RUN make
RUN make install

WORKDIR /app/build
RUN cmake ..
RUN make

ENTRYPOINT ["./user-app"]
