#include "structure.hpp"

unsigned int parseSize(const std::string& sizeStr) {
  unsigned long long size = 0;
  int multiplier = 1;
  size_t i = 0;

  for (; i < sizeStr.size() && std::isdigit(sizeStr[i]); ++i)
    ;

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
    return static_cast<size_t> UINT_MAX;
  }
  return static_cast<size_t>(result);
}


std::string toLower(const std::string& str) {
    std::string lowerStr = str;
    std::transform(lowerStr.begin(), lowerStr.end(), lowerStr.begin(),
                   static_cast<int(*)(int)>(std::tolower)); 
    return lowerStr;
}


bool parseConfig(std::vector<config_t>& serv,const std::string& filename) {
  std::ifstream configFile(filename.c_str());
  if (!configFile.is_open()) {
    std::cerr << "Failed to open file: " << filename << std::endl;
    return false;
  }

  config_t currentConfig;
  location_t currentLocation;
  bool inLocation = false;
//   bool listenSet = false;
//   bool serverNameSet = false;
 std::string line, accumulated;

  while (std::getline(configFile, line)) {
    
    std::istringstream iss(line);
    std::string key;
    iss >> key;

    if (key == "server") {
    char nextChar = iss.get();
      while (iss && isspace(nextChar)) nextChar = iss.get(); 
      if (nextChar != '{' || iss.get() != EOF) { 
        std::cerr << "Server config not found" << line << "'" << std::endl;
        return false;
      }
    //    listenSet =false;
    //   serverNameSet =false;
      currentConfig = config_t();
     
    } else if (key == "location") {
      inLocation = true;
      currentLocation =   location_t();
      iss >> currentLocation.name;
    } else if (key == "}") {
      if (inLocation) {
        inLocation = false;
        currentConfig.locations.push_back(currentLocation);
      } else {
        serv.push_back(currentConfig);
        // listenSet =false;
    //   serverNameSet =false;
      }
    } else if (inLocation) {
      if (key == "root") {
            std::string rootValue;
    iss>>rootValue;
    iss >> std::ws; 
    if (iss.peek() != EOF) { 
        std::cerr << "Invalid 'root' format: Only one argument expected." << std::endl;
        return false;
    }
    if (!currentLocation.root.empty()) {
        std::cerr << "Duplicate 'root' directive in location." << std::endl;
        return false;
    }
    currentLocation.root = rootValue;
      } else if (key == "allowed_method") {
      std::string method;
      std::map<std::string, methodID> validMethods;
      validMethods["get"] = GET;
      validMethods["post"] = POST;
      validMethods["delete"] = DELETE;
  
    while (iss >> method) {
        std::string lowerMethod = toLower(method);
        std::map<std::string, methodID>::iterator it = validMethods.find(lowerMethod);
        if (it == validMethods.end()) {
            std::cerr << "Invalid method: " << method << ". Only GET, POST, DELETE are allowed." << std::endl;
            return false;
        }
        currentLocation.allow[it->second] = true;
    }
    }
      } else if (key == "autoindex") {
        std::string value;
        iss >> value;
        currentLocation.atidx = (value == "on");
      } else if (key == "index") {
        std::string indexFile;
        while (iss >> indexFile) {
          currentLocation.indexFiles.push_back(indexFile);
        }
      }
    else {
      if (key == "listen") {
        //   if (listenSet) {
        //   std::cerr << "Duplicate 'listen' directive in config: " << line << std::endl;
        //   return false;
        // }
        if (currentConfig.listen != 0) {  
        std::cerr << "Duplicate 'listen' directive in config: " << line << std::endl;
        return false;
    }
    int port;
    if (!(iss >> port)) {  
        std::cerr << "Invalid 'listen' format, should be a single number: " << line << std::endl;
        return false;
    }
    iss >> std::ws;  
    if (iss.peek() != EOF) {
        std::cerr << "Invalid 'listen' format, should be a single number: " << line << std::endl;
        return false;
    }
    currentConfig.listen = port;
        // listenSet = true;  
      } else if (key == "server_name") {
    //     if (serverNameSet) {
    //     std::cerr << "Duplicate 'server_name' directive in config: " << line << std::endl;
    //     return false;
    //    }
    if (!currentConfig.name.empty()) {
    std::cerr << "Duplicate 'server_name' directive in config: " << line << std::endl;
    return false;
}
        std::string serverName;
        if (!(iss >> serverName)) {
        std::cerr << "Invalid 'server_name' format, should be a single name: " << line << std::endl;
        return false;
        }
        iss >> std::ws; 
        if (iss.peek() != EOF) {  
        std::cerr << "Invalid 'server_name' format, should be a single name: " << line << std::endl;
        return false;
    }
        currentConfig.name = serverName;
    // serverNameSet = true;
      } else if (key == "Client_body_size") {
        std::string clientbody;
        if (!(iss >> clientbody)) {
        std::cerr << "No value provided for 'Client_body_size'." << std::endl;
        return false; 
    }
    iss >> std::ws;
    if (iss.peek() != EOF) {
        std::cerr << "Invalid 'Client_body_size' format: Expected a single value." << std::endl;
        return false;
    }

    if (currentConfig.clientBodySize != 0) {
        std::cerr << "Duplicate 'Client_body_size' directive." << std::endl;
        return false; 
    }
        currentConfig.clientBodySize = parseSize(clientbody);
      }
    //   } else if (key == "error_pages") {
    //     std::string errorPages;
    // if (!(iss >> errorPages)) { 
    //     std::cerr << "Invalid 'error_pages' format: No value provided." << std::endl;
    //     return false;
    // }
    
    // iss >> std::ws; 
    // if (iss.peek() != EOF) {
    //     std::cerr << "Invalid 'error_pages' format: Should be a single path." << std::endl;
    //     return false;
    // }

    // currentConfig.errorPages = errorPages;
    // } 
    // else if (key == "Worker_connections") {
    //     iss >> currentConfig.workerConnections;
    //   }

    else if (key == "file40x") {
        iss >> currentConfig.file40x;
      } else if (key == "file50x") {
        iss >> currentConfig.file50x;
      }
    }
  }

  return true;
}

// void print(std::vector<config_t> serv)  {
//   for (size_t i = 0; i < serv.size(); ++i) {
//     const config_t& config = serv[i];
//     std::cout << "Server listen: " << config.listen << "\n"
//               << "Server name: " << config.name << "\n"
//               << "Client body size: " << config.clientBodySize << "\n";
//               // << "Error pages: " << config.errorPages << "\n";
//             //   << "Worker connections: " << config.workerConnections << "\n";
//     for (const  location_t& loc : config.locations) {
//       std::cout <<    "location_t name: " << loc.name << "\n"
//                 <<    "location_t root: " << loc.root << "\n"
//                 << "Autoindex: " << (loc.atidx ? "on" : "off") << "\n"
//                 << "Allowed methods: ";
//       for (const std::string& method : loc.allow) {
//         std::cout << method << " ";
//       }
//       std::cout << "\nIndex files: ";
//       for (const std::string& file : loc.indexFiles) {
//         std::cout << file << " ";
//       }
//       std::cout << "\n--------------------------------\n";
//     }
//     std::cout<<i<<"--server------------------finsished"<<std::endl;
//   }
// }



// int main() {
//   std::vector<config_t> myServer;
//   if (parseConfig(myServer,"default1.config")) {
//     print(myServer);
//   } else {
//     std::cerr << "Error parsing configuration file." << std::endl;
//   }
//   return 0;
// }