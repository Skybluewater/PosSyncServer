//
//  main.cpp
//  SocketServer
//
//  Created by 崔程远 on 2023/5/20.
//


// Server side implementation of UDP client-server model


#include <stdio.h>
#include <iostream>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <cstring>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <vector>
#include <atomic>

#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h>

#include <thread>
#include <fstream>
#include <map>
#include <set>


#include "ServerThread.hpp"

using namespace std;

map<int, int> recv_m = {
    {1, 4000},
    {2, 4001},
    {3, 4002},
    {4, 4003},
    {5, 4004}
};

int main() {
    ServerThread* t_arr[recv_m.size()];
    int cnt = 0;
    thread* pthread_arr[recv_m.size() * 2];
    
    for(auto &i: recv_m) {
        t_arr[cnt] = new ServerThread(i.first, i.second);
        pthread_arr[cnt * 2] = new thread(ServerThread::receiving, t_arr[cnt]);
        pthread_arr[cnt * 2 + 1] = new thread(ServerThread::update_map, t_arr[cnt]);
        cnt++;
    }
    
    for (auto &i: pthread_arr) {
        i->detach();
    }
    
    char c;
    cin>>c;
    for (int i = 0; i < cnt; i++) {
        ServerThread::stop(t_arr[i]);
        // while (!ServerThread::has_stopped(t_arr[i])) {
        delete t_arr[i];
    }
}
