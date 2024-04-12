#ifndef CLIENT_HPP
#define CLIENT_HPP

#include "ASocket.hpp"
#include "Server.hpp"

#define max 1024
typedef std::runtime_error err_t;
class Server;

class Client : ASocket {
  private:
    std::map<int, std::string> clients;
    Server &srv;

    char buf[max];

  public:
    Client(Server &);
    ~Client();

    const Server &server(void) const { return srv; }
    void setSocket(const socket_t &socket) { client_socket = socket; }
    const socket_t &socket(void) const { return client_socket; }

    void disconnect_client(int);

    // bool changeProperty(int);
    void processClientRequest(int, std::map<int, std::string> &, Server &);
    void handleChunkedRequest(int, std::map<int, std::string> &);
    void handleRegularRequest(int, std::map<int, std::string> &);

    const std::map<int, std::string> &getClients() const;
    const Server &getserver(void) const;
    const std::string getBufferContents() const { return std::string(buf); }
};

#endif