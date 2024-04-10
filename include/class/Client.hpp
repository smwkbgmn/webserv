#ifndef CLIENT_HPP
#define CLIENT_HPP

#include "ASocket.hpp"
#include "Server.hpp"

#define max 1024
typedef std::runtime_error err_t;

class Client : ASocket {
  private:
    std::map<int, std::string> clients;
    Server &server;

    char buf[max];

  public:
    Client(Server &);
    ~Client();

    void disconnect_client(int);

    // bool changeProperty(int);
    void processClientRequest(int, std::map<int, std::string> &);
    void handleChunkedRequest(int, std::map<int, std::string> &);

    void handleRegularRequest(int, std::map<int, std::string> &);
    const std::map<int, std::string> &getClients() const;
    const Server &getserver(void) const;
};

#endif