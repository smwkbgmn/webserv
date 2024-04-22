#ifndef CLIENT_HPP
# define CLIENT_HPP

# include "ASocket.hpp"
# include "Server.hpp"

# define SIZE_BUF 3000

class Client : ASocket {
  private:
    std::map<int, std::stringstream>& clients;
    char* msg;
    Server &srv;



    // std::vector<config_t> nginxConfigs;

    std::string sav_msg;

  public:
    char buf[SIZE_BUF];
    ssize_t byte_read;

    Client(Server &, std::map<int, std::stringstream>& );
    ~Client();


  // const std::vector<config_t>& getServerConfigs() const { return nginxConfigs; }
  // void setServerConfigs(const std::vector<config_t>& configs { nginxConfigs = configs; }

    // const char* buffer( void ) const { return buf; }
    // const char* buffer( void ) const { return clients.at( client_socket ).c_str; }
    // const char* buffer( void ) const { char* copied = NULL; memcpy( copied, clients.at( client_socket ).str().c_str(), byte_read ); return copied; }
    const char* buffer( void ) const { return msg; }
    // const std::map<int, std::stringstream>& strbuf( void ) const { return clients; }
    const Server &server(void) const { return srv; }
    void setSocket(const socket_t &socket) { client_socket = socket; }
    const socket_t &socket(void) const { return client_socket; }
    

    void disconnect_client(int);

    // bool changeProperty(int);
    void processClientRequest(int, std::map<int, std::stringstream> &, osstream_t&, size_t&, size_t& );
    // void handleChunkedRequest(int, std::map<int, std::stringstream> &);
    void handleRegularRequest(int, std::map<int, std::stringstream> &, osstream_t&, size_t&, size_t& );

    const std::map<int, std::stringstream> &getClients() const;
    const Server &getserver(void) const;
    const std::string getBufferContents() const { return std::string(buf); }
    // bool isRequestComplete(const std::string& request, size_t&, size_t& );
    bool isRequestComplete(const char*, const size_t&, size_t&, size_t& );
    void processFullRequest(int fd, const std::string& request);
};

#endif