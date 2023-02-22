#include <map>
#include <string>
#include <set>
#include <iostream>
#include <cstdio>
#include <cstdlib>
#include <cerrno>
#include <cstring>
#include <stdio.h>
#include <random>
#include <cassert>

#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <unistd.h>
#include <arpa/inet.h>

#include "precompute.hpp"
#include "Types.hpp"
#include "node.hpp"
#include "glog/logging.h"

using namespace std;


class client
{
private:
    /* data */
public:
    client(int maxSize, bytes<Key> k1, bytes<Key> k2, bytes<Key> k3);
    ~client();
    bool init();
    void insert_map(string key, int value);
    int search_map(string key);

    bool insert(string key, int value);
    kvpair* search(string key);
    void part_init();
    void finish(bool find);

    midNode1* Readmid1Node(string key, int leaf);
    midNode2* Readmid2Node(string key, int leaf);
    leafNode* ReadleafNode(string key, int leaf);
    int Writemid1Node(bool incache, string bid, midNode1* node);
    int Writemid2Node(bool incache, string bid, midNode2* node);
    int WriteleafNode(bool incache, string bid, leafNode* node);
    void Fetchmid1Path(int leaf);
    void Fetchmid2Path(int leaf);
    void FetchleafPath(int leaf);
    void Writemid1Path(int leaf, int d);
    void Writemid2Path(int leaf, int d);
    void WriteleafPath(int leaf, int d);
    void finalizemid1(bool find);
    void finalizemid2(bool find);
    void finalizeleaf(bool find);
    void finalize2mid1();
    void finalize2mid2();
    void finalize2leaf();
    std::vector<string> GetIntersectingBlocks1(int x, int curDepth);
    std::vector<string> GetIntersectingBlocks2(int x, int curDepth);
    std::vector<string> GetIntersectingBlocks3(int x, int curDepth);
    int RandomPath(int mt);
    int GetNodeOnPath(int maxSize, int leaf, int curDepth);
    block SerialiseBucket(Bucket bucket);

    template <class Node>
    Bucket DeserialiseBucket(block buffer);
    template <class Node>
    Node* convertBlockToNode(block b);
    template <class Node>
    block convertNodeToBlock(Node* node);

    rootNode *root;
    std::map<string, midNode1*> mid1cache;
    std::map<string, midNode2*> mid2cache;
    std::map<string, leafNode*> leafcache;
    std::set<string> mid1modified;
    std::set<string> mid2modified;
    std::set<string> leafmodified;
    std::vector<int> leafList1;
    std::vector<int> leafList2;
    std::vector<int> leafList3;
    std::vector<int> writeviewmap1;
    std::vector<int> writeviewmap2;
    std::vector<int> writeviewmap3;
    bytes<Key> key1;
    bytes<Key> key2;
    bytes<Key> key3;
    int readCntmid1;
    int readCntmid2;
    int readCntleaf;

    // Randomness
    std::random_device rd;
    std::mt19937 mt;
    std::uniform_int_distribution<int> dis;

    int clientSock;
    struct sockaddr_in serverAddr;
};

