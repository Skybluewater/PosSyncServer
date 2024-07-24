//
//  Server.hpp
//  SocketServer
//
//  Created by 崔程远 on 2023/5/20.
//

#ifndef Server_hpp
#define Server_hpp

#include <stdio.h>
#include <vector>
#include <netinet/in.h>
#include <atomic>
#include <string>
#include <mutex>
#include <map>

#include "UserStruct.h"

class ServerThread {
private:
    int id;
    int recvPort;
    std::vector<UserStruct> tempVec;
    int recvSock;
    std::atomic_bool running;
    std::atomic_bool sending_closed;
    std::atomic_bool receiving_closed;
    std::map<int, long> timeMap;
    std::map<int, int> structMap;
    std::vector<UserStruct> infos;
    std::mutex mut;
    std::map<int, int> portMap;
    //收端口、发端口
    std::map<int, int> recv_send_port;

public:
    ServerThread();
    ServerThread(int id, int port);
    ~ServerThread();
    
    static void receiving(ServerThread*);
    static void update_map(ServerThread*);
    
    static void stop(void*);
    static bool has_stopped(void*);
};

#endif /* Server_hpp */
