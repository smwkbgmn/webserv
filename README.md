# webserv

A custom HTTP/1.1 web server written in C++11, inspired by nginx. Built around a non-blocking, event-driven architecture using BSD kqueue.

## Features

- HTTP/1.1 compliant request parsing and response generation
- Non-blocking I/O with kqueue event loop
- Virtual host support (multiple server blocks on the same port)
- Location-based routing with URI rewriting
- Static file serving with auto-generated directory listings
- CGI/1.1 support (GET and POST)
- File uploads via POST
- Chunked transfer encoding
- Configurable error pages, timeouts, and max body size
- Supported methods: `GET`, `POST`, `DELETE`

## Requirements

- macOS (uses BSD kqueue)
- C++11 compatible compiler (`clang++` / `g++`)

## Build

```sh
make        # build
make run    # build and run with default config
make re     # clean rebuild
make fclean # remove binary, objects, and logs
```

The output binary is `./webserv`.

## Usage

```sh
./webserv [config_file]
```

If no config file is provided, `conf/default.conf` is used.

## Configuration

Configuration syntax is similar to nginx. Multiple `server` blocks are supported.

```nginx
server {
    listen          8080
    server_name     example.com

    client_max_body 30M
    file50x         /html/50x.html
    file40x         /html/40x.html

    location / {
        root            /html
        allowed_method  GET POST DELETE
        index           index.html
        index_auto      off
    }

    location /cgi-bin {
        root            /cgi-bin
        allowed_method  GET POST
        cgi             on
    }

    location /upload {
        root            /html/upload
        allowed_method  POST
        upload          /html/upload
    }

    location /old {
        rewrite         /new
    }
}
```

### Directives

| Directive | Scope | Description |
|---|---|---|
| `listen` | server | Port to listen on |
| `server_name` | server | Hostname(s) for virtual hosting |
| `client_max_body` | server | Max request body size (e.g. `10M`, `512K`) |
| `file50x` | server | Path to custom 5xx error page |
| `file40x` | server | Path to custom 4xx error page |
| `root` | location | Directory to serve files from |
| `allowed_method` | location | Space-separated list: `GET`, `POST`, `DELETE` |
| `index` | location | Default file for directory requests |
| `index_auto` | location | Auto-generate directory listing (`on`/`off`) |
| `cgi` | location | Enable CGI execution (`on`/`off`) |
| `upload` | location | Directory to store uploaded files |
| `rewrite` | location | Redirect requests to another URI |

## Project Structure

```
webserv/
├── conf/           # Configuration files
├── src/
│   ├── main.cpp
│   ├── common/     # Sockets, kqueue wrapper, config parser, utilities
│   ├── core/       # Webserv orchestrator, Server, Client
│   └── http/       # Request, Response, Transaction, CGI module
└── Makefile
```

## Architecture

The server runs a single-threaded event loop backed by kqueue:

1. **Webserv** — initializes the event loop, loads config, and manages `Server` and `Client` instances
2. **Server** — listens on a port and accepts new connections
3. **Client** — owns a connection; buffers I/O and tracks an active `Transaction`
4. **Transaction** — parses the `Request`, runs it through the matching location rules, and builds the `Response`
5. **CGI** — forks a child process, sets up the CGI environment, and pipes the request body in / response body out
