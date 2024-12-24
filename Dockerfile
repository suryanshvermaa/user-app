FROM drogonframework/drogon AS builder

WORKDIR /app

COPY . .
RUN mkdir -p build

WORKDIR /app/build

RUN cmake ..
RUN make

ENTRYPOINT ["./user-app"]
