FROM alpine:latest
WORKDIR /app
RUN apk add --no-cache build-base
COPY . .
RUN make build/main \
    && rm -rf build/obj .dockerignore Dockerfile include Makefile src
ENTRYPOINT ["./build/main"]
CMD ["-w"]