#include "Server.hpp"
#include "HTTP.hpp"

Server::Server() : ASocket(), EventList(8), client_event("client"), server_event("serv") {
    kq = kqueue();
    if (kq == -1)
        throw err_t("Failed to create kqueue");
    timeout.tv_sec = 120;
    timeout.tv_nsec = 0;

    confs.push_back( config_t() );
}

Server::~Server() {
    for (ConnectClients::iterator it = ClientMap.begin(); it != ClientMap.end(); ++it) {
        delete it->second;
        it->second = NULL;
    }
    for (size_t i = 0; i < EventList.size(); ++i) {
        close(EventList[i].ident);
    }
    close(kq);
}

void Server::DisconnectClient(int client_fd) {
    std::map<int, Client*>::iterator it = ClientMap.find(client_fd);
    if (it != ClientMap.end()) {
        close(client_fd);
        delete it->second;
        ClientMap.erase(it);
        // std::cout << "delet completed " << client_fd << std::endl;
    } else {
        std::cout << "Client not found in map: " << client_fd << std::endl;
    }
}


struct kevent &Server::getEventList(int idx) {
    return EventList[idx];
}

int Server::eventOccure() {
    int occure;
    occure = kevent(this->kq, NULL, 0, &EventList[0], 8, &timeout);
    if (occure == -1)
        throw err_t("Failed to make event");
    return occure;
}


bool Server::errorcheck(struct kevent &kevent) {
    if (kevent.flags & EV_ERROR) {
        // std::cerr << "EV_ERROR detected on descriptor " << kevent.ident << std::endl;
        // close(kevent.ident);
        DisconnectClient(kevent.ident);
        return true;
    }
    if ((kevent.flags & EV_EOF) && (kevent.filter != EVFILT_PROC)) {
        std::map<int, Client*>::iterator it = ClientMap.find(kevent.ident);
        if (it != ClientMap.end()) {
        if (!it->second->getCgiCheck())
            // std::cerr << "EV_EOF detected on descriptor " << kevent.ident << std::endl;
            // close(kevent.ident);
            DisconnectClient(kevent.ident);
            return true;
        }
    }
    return false;
}


void Server::ServerPreset() {
    openSocket();
    add_events(server_socket, EVFILT_READ, EV_ADD | EV_ENABLE, 0, 0, &server_event);
}

void Server::add_events(uintptr_t ident, int16_t filter, uint16_t flags,
                           uint32_t fflags, intptr_t data, void *udata) {
    struct kevent temp_event;
    EV_SET(&temp_event, ident, filter, flags, fflags, data, udata);
    kevent(kq, &temp_event, 1, NULL, 0, NULL);
}


void Server::handleNewConnection() {
    int client_fd = accept(server_socket, NULL, NULL);
    if (client_fd == -1) {
        std::cout << "check accept status" << std::endl;
        return;
    }
    Client* newClient = new Client(*this);
    newClient->setSocket(client_fd);
    this->setNonBlocking(newClient->getSocket());
    ClientMap[client_fd] = newClient;
    add_events(client_fd, EVFILT_READ, EV_ADD | EV_ENABLE, 0, 0, &client_event);
    add_events(client_fd, EVFILT_TIMER, EV_ONESHOT, 0, 30000, &client_event);
}

void Server::handleClientEvent(struct kevent &occur_event) {
    ConnectClients::iterator it = ClientMap.find(occur_event.ident);
    if (it != ClientMap.end()) {
        try {
            it->second->processClientRequest();
        } catch (const err_t& e) {
            std::clog<<"111111111111\n";
            DisconnectClient(it->second->getSocket());
        }
    } 
}

void Server::handleCGIEvent(struct kevent &occur_event) {
    int client = *(static_cast<int*>(occur_event.udata));
    ConnectClients::iterator it = ClientMap.find(client);
	Client& cl = *it->second;

    try {
		// IF CGI DONE
		CGI::readFrom( cl.subprocs, cl.out.body );
		CGI::build( cl.out );

        if (WEXITSTATUS(cl.subprocs.stat) != EXIT_SUCCESS) {
            throw errstat_t(500, "the CGI failed to exit as SUCCESS");

        }
		cl.in.reset();
		cl.subprocs.reset();


        if (it->second->getCgiCheck())it->second->setCgiCheck(FALSE);
        if (it->second->getCgiExit())it->second->setCgiExit(FALSE);
        add_events(it->second->get_process().pid, EVFILT_TIMER, EV_DELETE, 0, 0, NULL);
        add_events(client, EVFILT_WRITE, EV_ADD | EV_ONESHOT, 0, 0, NULL);
    } catch (const errstat_t& e) {
        std::cerr << "CGI error: " << e.what() << std::endl;
        
		Transaction::buildError( e.code, cl );
		// write_event;

        DisconnectClient(client);
    } 
}


void Server::handleReadEvent(struct kevent &occur_event) {
    char *check_type = static_cast<char*>(occur_event.udata);
    if (std::strcmp(check_type, "serv") == 0) 
        handleNewConnection();
    else if (std::strcmp(check_type, "client") == 0) 
        handleClientEvent(occur_event);
    else 
        handleCGIEvent(occur_event);
}

void Server::handleWriteEvent(struct kevent& event) {
    ConnectClients::iterator it = ClientMap.find(event.ident);
    
    if (it != ClientMap.end()) {
        if (!it->second->sendData()) {
            DisconnectClient(it->second->getSocket());
        } else {
            std::ostringstream msg;
            msg << "Data sent successfully to client: " << event.ident;
            // log(msg.str());
        }
    } else {
        std::ostringstream msg;
        msg << "Client not found in map for write event: " << event.ident;
    }
}


void Server::handleProcessExitEvent(struct kevent& event) {
    int client = *(static_cast<int *>(event.udata));
    ConnectClients::iterator it = ClientMap.find(client);

    if (it != ClientMap.end()) {
        process_t& procs = it->second->get_process();
        CGI::wait(procs);
        it->second->setCgiExit(TRUE);
        add_events(procs.fd[R], EVFILT_READ, EV_ADD | EV_ENABLE, 0, 0, it->second->get_client_socket_ptr());
        // add_events(procs.fd[R], EVFILT_TIMER, EV_ONESHOT, 0,30000, it->second->get_client_socket_ptr());
    }
}


void Server::handleTimerEvent(struct kevent& event) {
    ConnectClients::iterator it;
    int check_client = *(static_cast<int *>(event.udata));

    // log("Timer expired: ");
    it = ClientMap.find(event.ident);
    if (it != ClientMap.end()) {    
        std::clog <<"this is time eror\n";
        std::clog<<"55555555555555555555555\n";
        DisconnectClient(event.ident); 
    }
    else 
    {
        it = ClientMap.find(check_client);
        if (it != ClientMap.end()) {
            if (it->second->getCgiCheck()) {
                if (it->second->getCgiExit())
                {
                    close(event.ident);
                    DisconnectClient(check_client);
                    it->second->setCgiCheck(FALSE);
                    it->second->setCgiExit(FALSE);
                }
                else{ 
                    kill(event.ident, SIGTERM);
                    it->second->setCgiCheck(FALSE);
                    it->second->setCgiExit(FALSE); 
                    std::ostringstream msg;
                    msg << "Process terminated due to timeout: PID=" << event.ident;
                }
            }
        }
    }
            
}

void Server::connectsever() {
    ServerPreset();
    while (true) {
        int newEvent = eventOccure();
        for (int i = 0; i < newEvent; ++i) {
            struct kevent &event = getEventList(i);
                 if (errorcheck(event)) {
                    continue;
                 }
                if (event.filter == EVFILT_READ) 
                    handleReadEvent(event);      
                else if (event.filter == EVFILT_WRITE)
                    handleWriteEvent(event);
                else if (event.filter == EVFILT_PROC) 
                    handleProcessExitEvent(event); 
                else if (event.filter == EVFILT_TIMER)
                    handleTimerEvent(event);
        }
    }
}
