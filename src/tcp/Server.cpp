#include "Server.hpp"

struct kevent& Server::getEventList(int idx) { return server_list[idx]; }

Server::Server(void) : ASocket() ,server_list(8){
  this->kq = kqueue();
  if (kq == -1) throw err_t("Fail to create kqueue");

  change_events(server_socket, EVFILT_READ, EV_ADD | EV_ENABLE, 0, 0, NULL);

}



Server::~Server(void) { close(server_socket); }


void Server::disconnect_client(int client_fd)
{
    std::cout << "client disconnected: " << client_fd << std::endl;
    close(client_fd);
}


void Server::change_events( uintptr_t ident, int16_t filter,
        uint16_t flags, uint32_t fflags, intptr_t data, void *udata)
{
    struct kevent temp_event;

    EV_SET(&temp_event, ident, filter, flags, fflags, data, udata);
    kevent(kq, &temp_event, 1, NULL, 0, NULL);
    std::cout << "server started" <<std::endl;
}



// void Server::connect_sever()
// {
//   int new_events;
  
//   while(1)
//   {

//     new_events = kevent(this->kq,NULL,0,&server_list[0],8,NULL);

//     if (new_events == -1)
//      throw err_t("Fail to Make event"); 
//     for(int i =0; i< new_events; i++)
//     {
//       cur_event= &getEventList(i);
//       if (cur_event->flags & EV_ERROR)
//       {
//         if (cur_event->ident == static_cast<uintptr_t>(server_socket))
// 	  {
// 		close(server_socket);
//     	throw err_t("Server socket Error");
// 	  }
// 	    else {
// 		  close(client_socket);
//           throw err_t("client socket Error");
//         }
//       }

//       else if (cur_event->filter == EVFILT_READ)
//       {
//         if (cur_event->ident == static_cast<uintptr_t>(server_socket))
//         { 
//           if ((client_socket = accept(server_socket, NULL, NULL)) == -1)
// 		  {
// 			    std::cout <<"check error\n\n"<<std::endl;
//           close(client_socket);
//           throw err_t("Fail to accept in client server"); 
// 		  }
// 		  std::cout << "accept new client: " << client_socket << std::endl;
//           fcntl(client_socket, F_SETFL, O_NONBLOCK);
          
//         change_events( client_socket, EVFILT_READ, EV_ADD | EV_ENABLE, 0, 0, NULL);
//         change_events( client_socket, EVFILT_WRITE, EV_ADD | EV_ENABLE, 0, 0, NULL);
//         clients[client_socket] = "";
//         }
//          else if (clients.find(cur_event->ident)!= clients.end())
//                 {
//                     /* read data from client */
//                     char buf[1024];
//                     int n = read(cur_event->ident, buf, sizeof(buf));

//                     if (n <= 0)
//                     {
//                         if (n < 0)
//                             std::cerr << "client read error!" << std::endl;
//                         disconnect_client(cur_event->ident, clients);
//                     }
//                     else
//                     {
//                         buf[n] = '\0';
//                         clients[cur_event->ident] += buf;
//                         std::cout << "received data from " << cur_event->ident << ": " << clients[cur_event->ident] << std::endl;
//                     }
//                 }
//       }
//       else if (cur_event->filter == EVFILT_WRITE)
//             {
               
//                 std::map<int, std::string>::iterator it = clients.find(cur_event->ident);
//                 if (it != clients.end())
//                 {
//                     if (clients[cur_event->ident] != "")
//                     {
//                       std::string filePath = "web.html"; 
//                       std::ifstream fileStream(filePath);

//                       std::string line;
//                       std::string tmp_str;
//                       while (std::getline(fileStream, line)) {
// 						tmp_str += line;
//                       }
// 						int n;
// 						n = write(cur_event->ident, tmp_str.c_str(), tmp_str.size());
// 						if (n == -1)
// 						{
// 							std::cerr << "client write error!" << std::endl;
// 							disconnect_client(cur_event->ident, clients);  
// 						}
// 						else
// 						{
// 							clients[cur_event->ident].clear();
// 							std::cout << "n: " << n << "\n\n"; 
// 						}

//                     }
//                 }
//             }
//     }
//   }
// }




void Server::connect_sever()
{

	   std::string webContent;
    std::ifstream fileStream("web.html");
    if (fileStream.is_open()) {
        webContent = std::string((std::istreambuf_iterator<char>(fileStream)),
                                 std::istreambuf_iterator<char>());
        fileStream.close();
    } else {
        std::cerr << "Failed to open web.html" << std::endl;
        return; // Exit if cannot read the file
    }
  int new_events;
  
  while(1)
  {

    new_events = kevent(this->kq,NULL,0,&server_list[0],8,NULL);

    if (new_events == -1)
     throw err_t("Fail to Make event"); 
    for(int i =0; i< new_events; i++)
    {
      cur_event= &getEventList(i);
      if (cur_event->flags & EV_ERROR)
      {
        if (cur_event->ident == static_cast<uintptr_t>(server_socket))
	  {
		   close(server_socket);
    	throw err_t("Server socket Error");
	  }
	    else {
		  close(client_socket);
          throw err_t("client socket Error");
        }
      }

      else if (cur_event->filter == EVFILT_READ)
      {
        if (cur_event->ident == static_cast<uintptr_t>(server_socket))
        { 
          if ((client_socket = accept(server_socket, NULL, NULL)) == -1)
		  {
			    std::cout <<"check error\n\n"<<std::endl;
          close(client_socket);
          throw err_t("Fail to accept in client server"); 
		  }
		  std::cout << "accept new client: " << client_socket << std::endl;
        if(fcntl(client_socket, F_SETFL, O_NONBLOCK)==-1){
          close(client_socket);
          throw err_t("Fail to open file"); 
        }
          
        change_events( client_socket, EVFILT_READ, EV_ADD | EV_ENABLE, 0, 0, NULL);
        change_events( client_socket, EVFILT_WRITE, EV_ADD | EV_ENABLE, 0, 0, NULL);
        clients[client_socket] = "";
        }
         else if (clients.find(cur_event->ident)!= clients.end())
                {
                    /* read data from client */
                    char buf[1024];
                    int n = read(cur_event->ident, buf, sizeof(buf));

                    if (n <= 0)
                    {
                        if (n < 0)
                            std::cerr << "client read error!" << std::endl;
                        disconnect_client(cur_event->ident);
                    }
                    else
                    {
                        buf[n] = '\0';
                        clients[cur_event->ident] += buf;
                        std::cout << "received data from " << cur_event->ident << ": " << clients[cur_event->ident] << std::endl;
                    }
                }
      }
      else if (cur_event->filter == EVFILT_WRITE)
            {
                std::map<int, std::string>::iterator it = clients.find(cur_event->ident);
                 if (it != clients.end() && !webContent.empty()) {
                    ssize_t bytes_written = write(cur_event->ident, webContent.c_str(), webContent.size());
                    if (bytes_written > 0) {
                        std::cout << "Sent web.html content to client: " << cur_event->ident << std::endl;
                    change_events(cur_event->ident, EVFILT_WRITE, EV_DELETE, 0, 0, NULL);
            
                    clients.erase(cur_event->ident);
                    } else {
                        close(client_socket);
                         throw err_t("Faile to write"); 
                    }
                }
            }
				
            }
    }
  }