#include <cstdlib>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>
#include <map>
#include <stdexcept>
#include <algorithm>
#include <climits>
#include <cctype>

// #include "../structure.hpp"
#define on true
#define off false

typedef std::string str_t;
typedef std::string path_t;
typedef unsigned int uint_t;
typedef std::vector<uint_t> vec_uint_t;
typedef std::string name_t;
typedef std::vector<name_t> vec_name_t;

enum method_e {
    GET,
    POST,
    DELETE
};

struct config_s;

typedef struct location_s {
    str_t alias;
    path_t root;
    vec_uint_t allow;
    vec_name_t index;
    bool index_auto;

    location_s(const config_s&) : index_auto(false) {}
} location_t;

typedef std::vector<location_t> vec_location_t;

typedef int port_t;

typedef struct config_s {
    str_t name;
    port_t listen;
    path_t root;
    name_t file_40x;
    name_t file_50x;
    size_t client_max_body;
    vec_location_t locations;

    config_s() : listen(0), client_max_body(0) {}
} config_t;

unsigned int parseSize(const std::string& sizeStr) {
    unsigned long long size = 0;
    int multiplier = 1;
    size_t i = 0;

    for (; i < sizeStr.size() && std::isdigit(sizeStr[i]); ++i);

    std::istringstream iss(sizeStr.substr(0, i));
    iss >> size;

    if (i != sizeStr.size()) {
        std::string suffix = sizeStr.substr(i);
        if (suffix == "K") {
            multiplier = 1024;
        } else if (suffix == "M") {
            multiplier = 1024 * 1024;
        } else if (suffix == "G") {
            multiplier = 1024 * 1024 * 1024;
        }
    }

    unsigned long long result = size * multiplier;
    if (result > static_cast<unsigned long long>(UINT_MAX)) {
        return static_cast<size_t>(UINT_MAX);
    }
    return static_cast<size_t>(result);
}

std::string toLower(const std::string& str) {
    std::string lowerStr = str;
    for (size_t i = 0; i < lowerStr.size(); ++i) {
        lowerStr[i] = static_cast<char>(std::tolower(lowerStr[i]));
    }
    return lowerStr;
}

void handleListen(std::istringstream& iss, config_t& currentConfig) {
    if (currentConfig.listen != 0) {
        throw std::runtime_error("Error: Duplicate listen");
    }
    int port;
    if (!(iss >> port) || iss.peek() != EOF) {
        throw std::runtime_error("Error: Invalid listen format");
    }
    currentConfig.listen = port;
}

void handleServerName(std::istringstream& iss, config_t& currentConfig) {
    if (!currentConfig.name.empty() || !(iss >> currentConfig.name) || iss.peek() != EOF) {
        throw std::runtime_error("Error: Invalid server_name format");
    }
}

void handleClientBodySize(std::istringstream& iss, config_t& currentConfig) {
    std::string clientbody;
    if (!(iss >> clientbody) || iss.peek() != EOF || currentConfig.client_max_body != 0) {
        throw std::runtime_error("Error: Invalid Client_body_size format");
    }
    currentConfig.client_max_body = parseSize(clientbody);
}

void handleFile40x(std::istringstream& iss, config_t& currentConfig) {
    if (!(iss >> currentConfig.file_40x) || iss.peek() != EOF) {
        throw std::runtime_error("Error: Invalid file40x format");
    }
}

void handleFile50x(std::istringstream& iss, config_t& currentConfig) {
    if (!(iss >> currentConfig.file_50x) || iss.peek() != EOF) {
        throw std::runtime_error("Error: Invalid file50x format");
    }
}

void handleRoot(std::istringstream& iss, location_t& currentLocation) {
    std::string rootValue;
    iss >> rootValue;
    iss >> std::ws;
    if (iss.peek() != EOF || !currentLocation.root.empty()) {
        throw std::runtime_error("Error in root directive");
    }
    if (!rootValue.empty() && rootValue[rootValue.size() - 1] == '/') {
        throw std::runtime_error("Error: Can't finish with '/'");
    }
    currentLocation.root = rootValue;
}

void handleAllowedMethod(std::istringstream& iss, location_t& currentLocation) {
    std::string method;
    std::map<std::string, method_e> validMethods;
    validMethods["get"] = GET;
    validMethods["post"] = POST;
    validMethods["delete"] = DELETE;

    while (iss >> method) {
        std::string lowerMethod = toLower(method);
        std::map<std::string, method_e>::iterator it = validMethods.find(lowerMethod);
        if (it == validMethods.end()) {
            throw std::runtime_error("Invalid method: " + method + ". Only GET, POST, DELETE are allowed.");
        }
        currentLocation.allow.push_back(it->second);
    }
}

void handleAutoindex(std::istringstream& iss, location_t& currentLocation) {
    std::string value;
    iss >> value;
    currentLocation.index_auto = (value == "on");
}

void handleIndex(std::istringstream& iss, location_t& currentLocation) {
    std::string indexFile;
    while (iss >> indexFile) {
        currentLocation.index.push_back(indexFile);
    }
}

void handleServerRoot(std::istringstream& iss, config_t& currentConfig) {
    std::string rootValue;
    iss >> rootValue;
    iss >> std::ws;
    if (iss.peek() != EOF || !currentConfig.root.empty()) {
        throw std::runtime_error("Error in root directive");
    }
    if (!rootValue.empty() && rootValue[rootValue.size() - 1] == '/') {
        throw std::runtime_error("Error: Can't finish with '/'");
    }
    currentConfig.root = rootValue;
}

bool validateLocationPath(const std::string& path) {
    return !path.empty() && path[0] == '/' && path[path.size() - 1] != '/';
}

std::string trim(const std::string& str) {
    size_t start = 0;
    while (start < str.size() && (str[start] == ' ' || str[start] == '\t')) {
        ++start;
    }
    size_t end = str.size();
    while (end > start && (str[end - 1] == ' ' || str[end - 1] == '\t')) {
        --end;
    }
    return str.substr(start, end - start);
}

void parseConfig(std::vector<config_t>& serv, const std::string& filename) {
    std::ifstream configFile(filename.c_str());
    if (!configFile.is_open()) {
        throw std::runtime_error("Failed to open file: " + filename);
    }

    config_t currentConfig;
    location_t currentLocation(currentConfig);
    bool inLocation = false;
    std::string line;

    while (std::getline(configFile, line)) {
        line = trim(line);
        
        if (line.substr(0, 6) == "server" && line.find("{") != std::string::npos) {
            std::string rest = line.substr(6);
            rest = trim(rest);
            if (rest != "{") {
                throw std::runtime_error("Server config block not opened correctly: " + line);
            }
            currentConfig = config_t();
            continue;
        }

        if (line.substr(0, 8) == "location") {
            size_t pos = line.find("{");
            if (pos == std::string::npos) {
                throw std::runtime_error("Location block not opened correctly: " + line);
            }

            std::string locationPath = trim(line.substr(8, pos - 8));
            std::string rest = trim(line.substr(pos));
            if (!validateLocationPath(locationPath) || rest != "{") {
                throw std::runtime_error("Location block not opened correctly: " + line);
            }
            inLocation = true;
            currentLocation = location_t(currentConfig);
            currentLocation.alias = locationPath;
            continue;
        }

        std::istringstream iss(line);
        std::string key;
        iss >> key;

        if (key.empty()) {
            continue;
        }

        if (key == "}") {
            if (inLocation) {
                inLocation = false;
                currentConfig.locations.push_back(currentLocation);
            } else {
                serv.push_back(currentConfig);
            }
        } else if (inLocation) {
            if (key == "root") {
                handleRoot(iss, currentLocation);
            } else if (key == "allowed_method") {
                handleAllowedMethod(iss, currentLocation);
            } else if (key == "index_auto") {
                handleAutoindex(iss, currentLocation);
            } else if (key == "index") {
                handleIndex(iss, currentLocation);
            } else {
                throw std::runtime_error("Unknown directive in location block: " + key);
            }
        } else {
            if (key == "listen") {
                handleListen(iss, currentConfig);
            } else if (key == "server_name") {
                handleServerName(iss, currentConfig);
            } else if (key == "Client_body_size") {
                handleClientBodySize(iss, currentConfig);
            } else if (key == "file40x") {
                handleFile40x(iss, currentConfig);
            } else if (key == "file50x") {
                handleFile50x(iss, currentConfig);
            } else if (key == "root") {
                handleServerRoot(iss, currentConfig);
            } else {
                throw std::runtime_error("Unknown directive in server block: " + key);
            }
        }
    }
}



// void print(const std::vector<config_t>& serv) {
//     for (size_t i = 0; i < serv.size(); ++i) {
//         const config_t& config = serv[i];
//         std::cout << "Server listen: " << config.listen << "\n"
//                   << "Server name: " << config.name << "\n"
//                   << "Client body size: " << config.client_max_body << "\n"
//                   << "File 40x: " << config.file_40x << "\n"
//                   << "File 50x: " << config.file_50x << "\n";

//         for (size_t j = 0; j < config.locations.size(); ++j) {
//             const location_t& loc = config.locations[j];
//             std::cout << "Location URL: " << loc.alias << "\n"
//                       << "Location root: " << loc.root << "\n"
//                       << "Autoindex: " << (loc.index_auto ? "on" : "off") << "\n"
//                       << "Allowed methods: ";

//             for (size_t k = 0; k < loc.allow.size(); ++k) {
//                 switch (loc.allow[k]) {
//                     case GET: std::cout << "GET "; break;
//                     case POST: std::cout << "POST "; break;
//                     case DELETE: std::cout << "DELETE "; break;
//                     default: break;
//                 }
//             }

//             std::cout << "\nIndex files: ";
//             for (size_t k = 0; k < loc.index.size(); ++k) {
//                 std::cout << loc.index[k] << " ";
//             }
//             std::cout << "\n--------------------------------\n";
//         }  
//         std::cout << i << "--server------------------finished" << std::endl;
//     }
// }



// int main() {
//     std::vector<config_t> myServer;
//     try {
//         parseConfig(myServer, "default1.config");
//         print(myServer);  // 필요한 경우 출력 함수를 추가하여 결과를 출력할 수 있습니다.
//     } catch (const std::exception& e) {
//         std::cerr << "Error parsing configuration file: " << e.what() << std::endl;
//     }
//     return 0;
// }
