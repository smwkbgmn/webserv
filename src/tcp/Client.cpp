// #include "Client.hpp"

// Client::Client(int sockServer) : ASocket(sockServer) {}
// Client::~Client(void) { close(server_socket); }

// void Client::receiving(void) {
//   char buf[1024] = {0};
//   ssize_t bytesRead = read(sock, buf, sizeof(buf));
//   if (bytesRead == ERROR) throw err_t("fail to read from socket");

//   //   Transaction trans(buf, sock);
//   logfile.fs << buf;
// }

// void Client::sending(void) {
//   const char* response =
//       "HTTP/1.1 200 OK\r\nContent-Length: 12\r\n\r\nHello, World!";
//   ssize_t bytesSent = send(sock, response, strlen(response), 0);

//   if (bytesSent == ERROR) {
//     throw err_t("fail to send response");
//   } else {
//     std::clog << "Response sent successfully." << std::endl;
//   }
// }
