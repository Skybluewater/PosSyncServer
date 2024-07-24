//
//  Server.cpp
//  SocketServer
//
//  Created by 崔程远 on 2023/5/20.
//

#include "ServerThread.hpp"

#include <stdio.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <iostream>
#include <vector>
#include <map>
#include <thread>
#include <mutex>
#include <atomic>
#include <vector>
#include <cstring>
#include <time.h>
#include <set>

#include "UserStruct.h"
#define BUFF_SIZE 1024
#define MAX_CAPACITY 500
#define TIMEOUT 30

using namespace std;

ServerThread::ServerThread() {
}

ServerThread::~ServerThread() {
    close(recvSock);
    cout<<"Destroying ServerThread"<<endl;
}

ServerThread::ServerThread(int id, int port) {
    this->id = id;
    recvPort = port;
    infos.reserve(sizeof(UserStruct) * MAX_CAPACITY);
    tempVec.reserve(sizeof(UserStruct) * MAX_CAPACITY);
    running = true;
    sending_closed = false;
    receiving_closed = false;
    socklen_t len = sizeof(sockaddr_in);
    
    // init recvSock
    // cout<<"Initing recv sock"<<endl;
    recvSock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if (recvSock == -1) {
        cout<<"Error receiving socket"<<endl;
        exit(EXIT_FAILURE);
    }
    struct sockaddr_in recvAddr;
    memset(&recvAddr, 0, sizeof(recvAddr));
    recvAddr.sin_family = AF_INET;
    recvAddr.sin_port = htons(recvPort);
    recvAddr.sin_addr.s_addr = INADDR_ANY;
    
    if(::bind(recvSock, (struct sockaddr*)&recvAddr, sizeof(recvAddr)) < 0){
        cout<<"Listening Bind Failed"<<endl;
        close(recvSock);
        exit(EXIT_FAILURE);
    }
    cout<<"server init finished, using port: "<<port<<endl;
}

void ServerThread::receiving(ServerThread *c) {
    // ServerThread *c = (ServerThread *)t;
    struct sockaddr_in client_addr;
    socklen_t client_struct_length = sizeof(client_addr);
    char client_message[100], message[10000];
    memset(client_message, '\0', sizeof(client_message));
    memset(message, '\0', sizeof(message));
    int n = 0;
    UserStruct uc;
    time_t localtime;
    long size;
    
    // run receive thread
    while (c->running) {
        // Receive client's message:
        n = recvfrom(c->recvSock, (char *)client_message, sizeof(client_message),
                         MSG_WAITALL, (struct sockaddr *)&client_addr,
                         &client_struct_length);
        if (n < 0) {
            printf("Receiving error\n");
        }
        printf("Received message from IP: %s and port: %i\n",
               inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port));

        memcpy(&uc, client_message, sizeof(UserStruct));
        
        cout<<"UserID: "<<uc.user_id<<endl;
        
        //TODO: fetch a thread from pool to send
        
        // insert into time
        time(&localtime);
        {
            std::lock_guard<std::mutex> my_guard(c->mut);
            bool first_added = false;
            auto it = c->timeMap.find(uc.user_id);
            if (it == c->timeMap.end()) {
                first_added = true;
                c->infos.emplace_back(uc);
                c->structMap[uc.user_id] = (int)(c->infos.size() - 1);
            }
            c->timeMap[uc.user_id] = (long)localtime;
            if (!first_added) {
                c->infos[c->structMap[uc.user_id]] = uc;
            }
            
            // 将 infos 的内容复制到 message
            size = c->infos.size() * sizeof(UserStruct);
            memcpy(message, (const char *)c->infos.data(), size);

            //接收到收端口发来的数据，创建portMap的映射
            if (uc.isTest)
            {
                c->portMap[uc.user_id] = client_addr.sin_port;
            }
            //接收到发端口发来的数据，创建recv_send_port的映射
        }
        // Get a thread from pool to send
        // Message::submit(c, client_addr);
        
        // Send Mesasge
        if (!uc.isTest && c->portMap[uc.user_id] == 0)
        {
            //发端口发来的消息，且收发端口为建立映射
        }
        else
        {
            //如果是收端口发来的消息，直接返回
            if (uc.isTest)
            {
                if (sendto(c->recvSock, message, size, MSG_WAITALL, (struct sockaddr*)&client_addr, sizeof(sockaddr_in)) < 0) {
                    printf("Can't send\n");
                }
                printf("Send message to IP: %s and port: %i\n",
                    inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port));
                // memset(client_message, '\0', sizeof(client_message));
            }
            //如果是发端口发来的消息，修改后返回
            else
            {
                client_addr.sin_port = (USHORT)(c->portMap[uc.user_id]);

                if (sendto(c->recvSock, message, size, MSG_WAITALL, (struct sockaddr*)&client_addr, sizeof(sockaddr_in)) < 0) {
                    printf("Can't send\n");
                }
                printf("Send message to IP: %s and port: %i\n",
                    inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port));
                // memset(client_message, '\0', sizeof(client_message));
            }
        }
    }
    c->receiving_closed = true;
}

void ServerThread::update_map(ServerThread *c) {
    time_t localtime;
    set<int> reserve_set, remove_set;
    while (c->running) {
        std::this_thread::sleep_for(std::chrono::milliseconds(30000));
        cout<<"Updating map"<<endl;
        time(&localtime);
        {
            std::lock_guard<std::mutex> my_guard(c->mut);
            if (!c->timeMap.empty()) {
                auto iter = c->timeMap.begin();
                while (iter != c->timeMap.end()) {
                    if (localtime - iter->second <= TIMEOUT) {
                        reserve_set.insert(iter->first);
                    } else {
                        remove_set.insert(iter->first);
                    }
                    iter++;
                }
            }
            for (auto it = remove_set.begin(); it != remove_set.end(); it++) {
                c->structMap.erase(*it);
                c->timeMap.erase(*it);
                c->recv_send_port.erase(c->portMap[*it]);
                c->portMap.erase(*it);
            }
            for (auto it = reserve_set.begin(); it != reserve_set.end(); it++) {
                c->tempVec.emplace_back(c->infos[c->structMap[*it]]);
                c->structMap[*it] = (int)(c->tempVec.size() - 1);
            }
            vector<UserStruct> *temp;
            temp = &c->infos;
            c->infos = c->tempVec;
            c->tempVec = *temp;
        }
        c->tempVec.clear();
        reserve_set.clear();
        remove_set.clear();
    }
}

void ServerThread::stop(void *p) {
    ServerThread* c = (ServerThread *) p;
    c->running = false;
}

bool ServerThread::has_stopped(void* p) {
    ServerThread* c = (ServerThread *) p;
    return c->sending_closed && c->receiving_closed;
}
