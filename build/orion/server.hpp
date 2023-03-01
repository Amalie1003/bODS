#ifndef SERVER_H
#define SERVER_H

#include <random>
#include <vector>
#include <unordered_map>
#include <string>
#include <iostream>
#include <map>
#include <set>
#include <bits/stdc++.h>
#include <time.h>
#include "ORAM.hpp"
#include "ORAM.cpp"

#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <unistd.h>
#include <arpa/inet.h>

#include "glog/logging.h"

using namespace std;


class server
{
private:
    /* data */
public:
    server();
    ~server();
    void init();

    ORAM<midNode1> *oram1;
    ORAM<midNode2> *oram2;
    ORAM<leafNode> *oram3;

    int listenfd;
    struct sockaddr_in addr;
    struct sockaddr_in clientAddr;
    int conn;
};




#endif
