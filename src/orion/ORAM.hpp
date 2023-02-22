#ifndef ORAM_H
#define ORAM_H

#include "AES.hpp"
#include <random>
#include <vector>
#include <unordered_map>
#include <string>
#include <iostream>
#include "RAMStore.hpp"
#include <map>
#include <set>
#include <bits/stdc++.h>
#include "node.hpp"

#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <unistd.h>
#include <arpa/inet.h>

#include "glog/logging.h"

using namespace std;

template <class Node>
class ORAM {
private:
    RAMStore* store;
    using Stash = std::unordered_map<string, block>;
    size_t depth;
    size_t blockSize;
    
    int readCnt = 0;


    size_t plaintext_size;
    size_t bucketCount;
    size_t clen_size;
    bool batchWrite = false;

    //bool WasSerialised();
    //void Print();
    int GetNodeOnPath(int leaf, int curDepth);
    block SerialiseBucket(Bucket bucket);


public:
    ORAM(int maxSize, bytes<Key> key);
    ~ORAM();
 
    void start();
    void FetchPath(int leaf);
    void WriteBucket(int pos, block bucket);
    int conn;
    bytes<Key> key;
    std::vector<int> readviewmap;
};

#endif 
