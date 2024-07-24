//
//  Message.h
//  SocketServerr2.0
//
//  Created by 崔程远 on 2023/5/26.
//

#ifndef Message_h
#define Message_h

#include <netinet/in.h>
#include <arpa/inet.h>
#include <vector>


#include "UserStruct.h"
#include "thread_pool.h"
#include "ServerThread.hpp"

#define PEER_PORT 4000

extern int sock_arr[];
extern ThreadPool *pool;
extern std::mutex sock_mut;
extern std::set<int> idle_socks;


namespace Message {
void send_message(ServerThread* c, sockaddr_in addr) {
    int sending_sock, sock_idx;
    if (idle_socks.size() == 0) {
        return;
    }
    {
        std::lock_guard<std::mutex> my_guard(sock_mut);
        sock_idx = *idle_socks.begin();
        idle_socks.erase(idle_socks.begin());
    }
    sending_sock = sock_arr[sock_idx];
    addr.sin_port = htons(PEER_PORT);
    long size;
    char message[10000];
    {
        std::lock_guard<std::mutex> my_guard(c->mut);
        size = c->infos.size() * sizeof(UserStruct);
        memcpy(message, (const char *)c->infos.data(), size);
    }
    //printf("Send message to IP: %s and port: %i\n", inet_ntoa(addr.sin_addr), ntohs(addr.sin_port));
    if (sendto(sending_sock, message, size, MSG_WAITALL, (struct sockaddr*)&addr, sizeof(sockaddr_in)) < 0) {
        printf("Can't send\n");
    }
    {
        std::lock_guard<std::mutex> my_guard(sock_mut);
        idle_socks.insert(sock_idx);
    }
}

void submit(ServerThread* c, sockaddr_in addr) {
    pool->submit(send_message, c, addr);
}
}

#endif /* Message_h */
