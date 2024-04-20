#ifndef CLIENT_HPP
# define CLIENT_HPP

# include "ASocket.hpp"
# include "Server.hpp"

class Client : ASocket {
  private:
    std::map<int, std::string> clients;
    Server &srv;
    std::vector<config_t> nginxConfigs;

    std::string sav_msg;

  public:
    char buf[max];

    Client(Server &);
    ~Client();


  const std::vector<config_t>& getServerConfigs() const { return nginxConfigs; }
  void setServerConfigs(const std::vector<config_t>& configs) { nginxConfigs = configs; }

    const char* buffer( void ) const { return buf; }
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
    bool isRequestComplete(const std::string& request);
    void Client::processFullRequest(int fd, const std::string& request);
};

#endif