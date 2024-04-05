#ifndef CGI_HPP
# define CGI_HPP

#include "ASocket.hpp"

class Cgi{
    private:
        std::vector<std::string> env_vector;
        pid_t pid;

    public:
    Cgi(char **);
    ~Cgi();
    pid_t execute(Cgi &);
};
#endif