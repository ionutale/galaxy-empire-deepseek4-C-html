FROM alpine:3.21 AS builder

RUN apk add --no-cache \
    gcc musl-dev make \
    libuv-dev postgresql-dev \
    openssl-dev

WORKDIR /app

COPY Makefile .
COPY src/ src/
COPY third_party/ third_party/

RUN make

FROM alpine:3.21

RUN apk add --no-cache \
    libuv libpq \
    libcrypto3 libssl3

COPY --from=builder /app/galaxy-empire-server /usr/local/bin/server
COPY migrations/ /app/migrations/

EXPOSE 8080

CMD ["/usr/local/bin/server"]
