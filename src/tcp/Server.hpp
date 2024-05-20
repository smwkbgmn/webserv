#ifndef SERVER_HPP
#define SERVER_HPP

#include "ASocket.hpp"
#include "Client.hpp"
#include "structure.hpp"

#define MAX_EVENTS 10
class Client;

typedef std::map<int, Client*> ConnectClients;
typedef int kque;
typedef std::vector<struct kevent> eventQueue;


class Server : public ASocket {
private:
    kque kq;
    eventQueue EventList;
    struct timespec timeout;
    ConnectClients ClientMap;
    char client_event[8];
    char server_event[8];

	// config_t	conf;
	const vec_config_t& confs;
    
public:
    Server( const vec_config_t& );
    ~Server();
    void add_events(uintptr_t, int16_t, uint16_t, uint32_t, intptr_t, void *);
    
    void connectsever();
    void DisconnectClient(int client_fd);

    void ServerPreset();
    int eventOccure();
    bool errorcheck(struct kevent &);
    
    void handleReadEvent(struct kevent &);
    void handleWriteEvent(struct kevent& );
    void handleProcessExitEvent(struct kevent&);
    void handleTimerEvent(struct kevent& );
    void handleCGIEvent(struct kevent &);
    void handleClientEvent(struct kevent &);
    void handleNewConnection();

    struct kevent &getEventList(int);

    char* getClientEvent() { return client_event; }
    char* getServerEvent() { return server_event; }

    // const vec_config_t& config( void ) const { return conf; }
	const vec_config_t&	config( void ) const { return confs; }
	const config_t& configDefault( void ) const { return confs.at( 0 ); }
};

#endif
