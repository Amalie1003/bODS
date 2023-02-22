#include "client.hpp"

client::client(int maxSize, bytes<Key> k1, bytes<Key> k2, bytes<Key> k3): rd(), mt(rd()), dis(0, pow(2, floor(log2(maxSize / Z))) - 1)
{
    AES::Setup();
    key1 = k1;
    key2 = k2;
    key3 = k3;
    std::vector<int> params = param_bODS(10*4);
    P1 = params[0];
    P2 = params[1];
    C = params[2];
    root = new rootNode();
}

client::~client()
{
    AES::Cleanup();
}

int client::search_map(string key)
{
    part_init();
    kvpair *ret = search(key);
    finish(true);
    return ret->value;
}

void client::insert_map(string key, int value)
{
    part_init();
    insert(key, value);
    finish(false);
}

bool client::init()
{

    clientSock = socket(AF_INET, SOCK_STREAM, 0);
    if (clientSock == -1) {
        LOG(ERROR) << "Error: socket";
        //std::cout << "Error: socket" << std::endl;
        return false;
    }
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(8000);
    serverAddr.sin_addr.s_addr = inet_addr("127.0.0.1");
    if (connect(clientSock, (struct sockaddr*)&serverAddr, sizeof(serverAddr)) < 0) {
        LOG(ERROR) << "Error: socket";
        //std::cout << "Error: connect" << std::endl;
        return false;
    }
    LOG(INFO) << "...connect";
    //std::cout << "...connect" << std::endl;
    return true;
}

void client::part_init()
{
    readCntmid1 = 0;
    readCntmid2 = 0;
    readCntleaf = 0;
    writeviewmap1.clear();
    writeviewmap2.clear();
    writeviewmap3.clear();
}

kvpair* client::search(string key)
{
    string m1_key, m2_key, l_key;
    int m1_pos = root->Search(key, m1_key);
    midNode1* m1 = Readmid1Node(m1_key, m1_pos);
    int m2_pos = m1->Search(key, m2_key);
    midNode2* m2 = Readmid2Node(m2_key, m2_pos);
    int l_pos = m2->Search(key, l_key);
    leafNode* l = ReadleafNode(l_key, l_pos);
    return l->Search(key);
}

bool client::insert(string key, int value)
{
    if (root->p1 == 0){
        LOG(INFO) << "---开始第一个数据插入---";
        leafNode *leaf = new leafNode(key, value);
        WriteleafNode(false, key, leaf);
        midNode2 *mid2 = new midNode2();
        mid2->Insert(leaf);
        Writemid2Node(false, mid2->max_value, mid2);
        midNode1 *mid1 = new midNode1();
        mid1->Insert(mid2);
        Writemid1Node(false, mid1->max_value, mid1);
        root->Insert(mid1);
        return true;
        // return rootKey;
        /*---code---*/
    }
    string m1_key, m2_key, l_key;
    int m1_pos = root->Search(key, m1_key);
    std::cout << "m1pos:" << m1_pos << std::endl;
    midNode1* m1 = Readmid1Node(m1_key, m1_pos);
    //if(mid1cache.count(m1_key) != 0) mid1cache.erase(m1_key);
    // if(m1 == NULL) std::cout << "wrx:" << std::endl;
    
    int m2_pos = m1->Search(key, m2_key);
    std::cout << "m2pos:" << m2_pos << std::endl;
    midNode2* m2 = Readmid2Node(m2_key, m2_pos);
    //if(mid2cache.count(m2_key) != 0) mid2cache.erase(m2_key);


    int l_pos = m2->Search(key, l_key);
    std::cout << "leafpos:" << l_pos << std::endl;
    leafNode* l = ReadleafNode(l_key, l_pos);
    //if(leafcache.count(l_key) != 0) leafcache.erase(l_key);

    leafNode* newNode = l->Insert(key, value);
    WriteleafNode(true, l->max_value, l);
    //bool flag = true;
    m2->Update(l);
    if(newNode != NULL)
    {
        WriteleafNode(false, newNode->max_value,newNode);
        midNode2* newNode2 = m2->Insert(newNode);       
        m1->Update(m2);
        if(newNode2 != NULL)
        {
            Writemid2Node(false, newNode2->max_value, newNode2);
            midNode1* newNode3 = m1->Insert(newNode2);          
            root->Update(m1);
            if(newNode3 != NULL)
            {
                Writemid1Node(false, newNode3->max_value, newNode3);
                /*flag = */root->Insert(newNode3);
            }
        }
        else root->Update(m1);
    }
    else{
        m1->Update(m2);
        root->Update(m1);
    }
    Writemid2Node(true, m2->max_value, m2);
    Writemid1Node(true, m1->max_value, m1);
    
    return true;
}

void client::finish(bool find)
{
    finalizemid1(find);
    finalizemid2(find);
    finalizeleaf(find);

    for (auto t : leafcache) {
        if (t.second != NULL) {
            leafNode* tmp = t.second;
            if (leafmodified.count(tmp->max_value)) {
                tmp->pos = RandomPath(P1 * P2 * P2);
                LOG(INFO) << "leafNode: pos=" << tmp->pos;
            }
        }
    }
    for(auto t: mid2cache){
        if(t.second != NULL){
            midNode2* tmp = t.second;
            if(mid2modified.count(tmp->max_value)){
                tmp->pos = RandomPath(P1 * P2);
                LOG(INFO) << "mid2Node: pos=" << tmp->pos;
            }
            for(int i = 0; i < tmp->p2; i++)
            {
                string temp_key = tmp->childMap[i]->key;
                if(leafcache.count(temp_key))
                {
                    tmp->childMap[i]->pos = leafcache[temp_key]->pos;
                }
            }
            for(std::vector<kppair*>::iterator it = tmp->childMap.begin(); it != tmp->childMap.end(); it++)
            {
                if(*it != NULL)
                    std::cout << (*it)->key << " " << (*it)->pos << std::endl;
                //std::cout << &(tmp->my_map.end()) << std::endl;
            }
        }
    }
    for(auto t: mid1cache)
    {
        if(t.second != NULL){
            midNode1* tmp = t.second;
            if(mid1modified.count(tmp->max_value)){
                tmp->pos = RandomPath(P1);
                LOG(INFO) << "mid1Node: pos=" << tmp->pos;
            }
            for(int i = 0; i < tmp->p2; i++)
            {
                string tmp_key = tmp->childMap[i]->key;
                if(mid2cache.count(tmp_key))
                {
                    tmp->childMap[i]->pos = mid2cache[tmp_key]->pos;
                }
            }
        }
    }
    for(int i = 0 ; i < root->p1; i++)
    {
        string tmp_key = root->childMap[i]->key;
        if(mid1cache.count(tmp_key))
        {
            root->childMap[i]->pos = mid1cache[tmp_key]->pos;
        }
    }
    LOG(INFO) << "-----mid1oram-finalize2-----";
    finalize2mid1();
    LOG(INFO) << "-----mid2oram-finalize2-----";
    finalize2mid2();
    LOG(INFO) << "-----leaforam-finalize2-----";
    finalize2leaf();
}

midNode1* client::Readmid1Node(string bid, int leaf) {
    if (bid == "") {
        return NULL;
    }
    if (mid1cache.count(bid) == 0) {
        midNode1* node;
        Fetchmid1Path(leaf);
        
        // ReadData
        if (mid1cache.find(bid) == mid1cache.end()) {
            node = NULL;
        }
        else node = mid1cache[bid];
        // Access
        if (node != NULL) {
            node->pos = leaf;
            if (mid1cache.count(bid) != 0) {
                mid1cache.erase(bid);
            }
            mid1cache[bid] = node;
            if (find(leafList1.begin(), leafList1.end(), leaf) == leafList1.end()) {
                leafList1.push_back(leaf);
            }
        }
        //
        if (node != NULL) {
            mid1modified.insert(bid);
        }
        return node;
    } 
    else 
    {
        mid1modified.insert(bid);
        midNode1* node = mid1cache[bid];
        //node->pos = newLeaf;
        return node;
    }
}

midNode2* client::Readmid2Node(string bid, int leaf) {
    if (bid == "") {
        return NULL;
    }
    if (mid2cache.count(bid) == 0) {
        midNode2* node;
        Fetchmid2Path(leaf);
        
        // ReadData
        if (mid2cache.find(bid) == mid2cache.end()) {
            node = NULL;
        }
        else node = mid2cache[bid];
        // Access
        if (node != NULL) {
            node->pos = leaf;
            if (mid2cache.count(bid) != 0) {
                mid2cache.erase(bid);
            }
            mid2cache[bid] = node;
            if (find(leafList2.begin(), leafList2.end(), leaf) == leafList2.end()) {
                leafList2.push_back(leaf);
            }
        }
        //
        if (node != NULL) {
            mid2modified.insert(bid);
        }
        return node;
    } 
    else {
        mid2modified.insert(bid);
        midNode2* node = mid2cache[bid];
        node->pos = leaf;
        return node;
    }
}

leafNode* client::ReadleafNode(string bid, int leaf) {
    if (bid == "") {
        return NULL;
    }
    if (leafcache.count(bid) == 0) {
        leafNode* node;
        FetchleafPath(leaf);
        
        // ReadData
        if (leafcache.find(bid) == leafcache.end()) {
            node = NULL;
        }
        else node = leafcache[bid];
        // Access
        if (node != NULL) {
            node->pos = leaf;
            if (leafcache.count(bid) != 0) {
                leafcache.erase(bid);
            }
            leafcache[bid] = node;
            if (find(leafList3.begin(), leafList3.end(), leaf) == leafList3.end()) {
                leafList3.push_back(leaf);
            }
        }
        //
        if (node != NULL) {
            leafmodified.insert(bid);
        }
        return node;
    } else {
        leafmodified.insert(bid);
        leafNode* node = leafcache[bid];
        node->pos = leaf;
        return node;
    }
}

template <class Node>
Bucket client::DeserialiseBucket(block buffer) {
    int blockSize = sizeof(Node);
    assert(buffer.size() == Z * (blockSize));

    Bucket bucket;

    for (int z = 0; z < Z; z++) {
        Block &block = bucket[z];

        block.data.assign(buffer.begin(), buffer.begin() + blockSize);
        Node* node = convertBlockToNode<Node>(block.data);
        block.id = node->max_value;
        //delete node;
        buffer.erase(buffer.begin(), buffer.begin() + blockSize);
    }

    return bucket;
}

template <class Node>
Node* client::convertBlockToNode(block b) {
    Node* node = new Node();
    std::array<byte_t, sizeof (Node) > arr;
    std::copy(b.begin(), b.begin() + sizeof (Node), arr.begin());
    from_bytes(arr, *node);
    return node;
}

template <class Node>
block client::convertNodeToBlock(Node* node)
{
    if(node != NULL){
        for(std::vector<kppair*>::iterator it2 = node->childMap.begin(); it2 != node->childMap.end(); it2++)
        {
            if(*it2 != NULL)
                std::cout << (*it2)->key << " " << (*it2)->pos << std::endl;
            //std::cout << &(node->my_map.end()) <<std::endl;
        }
    }
    std::array<byte_t, sizeof (Node) > data = to_bytes(*node);
    block b(data.begin(), data.end());
    return b;
}

int client::Writemid1Node(bool incache, string bid, midNode1* node) {
    if (bid == "") {
        throw runtime_error("Node id is not set");
    }
    if (!incache) {
        mid1modified.insert(bid);
        // Access(bid, node);
        //FetchPath(node->pos);
        Fetchmid1Path(node->pos);
        //WriteData(bid, node);
        mid1cache[bid] = node;
        //store->ReduceEmptyNumbers();
        if (find(leafList1.begin(), leafList1.end(), node->pos) == leafList1.end()) {
            leafList1.push_back(node->pos);
        }
        
        return node->pos;
    } 
    else {
        mid1modified.insert(bid);
        return node->pos;
    }
}

int client::Writemid2Node(bool incache, string bid, midNode2* node) {
    if (bid == "") {
        throw runtime_error("Node id is not set");
    }
    if (!incache) {
        mid2modified.insert(bid);
        // Access(bid, node);
        //FetchPath(node->pos);
        Fetchmid2Path(node->pos);
        //WriteData(bid, node);
        mid2cache[bid] = node;
        //store->ReduceEmptyNumbers();
        if (find(leafList2.begin(), leafList2.end(), node->pos) == leafList2.end()) {
            leafList2.push_back(node->pos);
        }
        return node->pos;
    } else {
        mid2modified.insert(bid);
        return node->pos;
    }
}

int client::WriteleafNode(bool incache, string bid, leafNode* node) {
    if (bid == "") {
        throw runtime_error("Node id is not set");
    }
    if (!incache) {
        leafmodified.insert(bid);
        // Access(bid, node);
        //FetchPath(node->pos);
        FetchleafPath(node->pos);
        //WriteData(bid, node);
        leafcache[bid] = node;
        //store->ReduceEmptyNumbers();
        if (find(leafList3.begin(), leafList3.end(), node->pos) == leafList3.end()) {
            leafList3.push_back(node->pos);
        }
        return node->pos;
    } else {
        leafmodified.insert(bid);
        return node->pos;
    }
}

void client::Fetchmid1Path(int leaf)
{
    readCntmid1++;
    // send & receive
    char data[255];
    memset(data, 0, sizeof(data));
    sprintf(data, "%d", leaf);
    data[strlen(data)] = '1';
    data[strlen(data)] = 0;
    send(clientSock, data, strlen(data), 0);
    LOG(INFO) << "[mid1]send and fetch mid1path:" << leaf; 
    //std::cout << "send mid1Path:" << leaf << std::endl;
    //Access(bid, node, leaf, leaf);
    unsigned char recv_buf[10240];
    memset(recv_buf, 0, 10240);
    int len = recv(clientSock, recv_buf, sizeof(recv_buf), 0);
    if(len == 1 && recv_buf[0] == '\0') {
        LOG(INFO) << "[mid1]receive NONE.";
        //std::cout << "receive none" << std::endl;
        return;
    }
    recv_buf[len] = '\0';
    LOG(INFO) << "[mid1]receive mid1oram bits:" << len;
    //std::cout << "receive mid1oram bits:" << len << std::endl;

    int i = 0;
    int blockSize = sizeof(midNode1) * Z;
    int clen = AES::GetCiphertextLength(blockSize);
    while (i * (clen + 16) < len)
    {
        //char*cipher = new char[blockSize];
        //memcpy(cipher, recv_buf + i * blockSize, blockSize);
        block ciphertext;
        ciphertext.insert(ciphertext.end(), recv_buf + i * (clen + 16), recv_buf + (i + 1) * (clen + 16));
        block buffer = AES::Decrypt(key1, ciphertext, clen);
        Bucket bucket = DeserialiseBucket<midNode1>(buffer);

        for (int z = 0; z < Z; z++) {
            Block &block = bucket[z];

            if (block.id != "") { // It isn't a dummy block   
                midNode1* n = convertBlockToNode<midNode1>(block.data);
                if (mid1cache.count(block.id) == 0) {
                    mid1cache.insert(make_pair(block.id, n));
                } else {
                    //delete n;
                }
            }
        }
        i++;
    }
}

void client::Fetchmid2Path(int leaf)
{
    readCntmid2++;
    // send & receive
    char data[255];
    memset(data, 0, sizeof(data));
    sprintf(data, "%d", leaf);
    data[strlen(data)] = '2';
    data[strlen(data)] = 0;
    send(clientSock, data, strlen(data), 0);
    LOG(INFO) << "[mid2]send and fetch mid2path:" << leaf; 
    //std::cout << "send mid2Path:" << leaf << std::endl;
    //Access(bid, node, leaf, leaf);
    unsigned char recv_buf[10240];
    memset(recv_buf, 0, 10240);
    int len = recv(clientSock, recv_buf, sizeof(recv_buf), 0);
    if(len == 1 && recv_buf[0] == '\0') {
        LOG(INFO) << "[mid2]receive NONE.";
        //std::cout << "receive none" << std::endl;
        return;
    }
    recv_buf[len] = '\0';
    LOG(INFO) << "[mid2]receive mid2oram bits:" << len;
    //std::cout << "receive mid2oram bits:" << len << std::endl;
    int i = 0;
    int blockSize = sizeof(midNode2) * Z;
    int clen = AES::GetCiphertextLength(blockSize);
    while (i * (clen + 16) < len)
    {
        //char*cipher = new char[blockSize];
        //memcpy(cipher, recv_buf + i * blockSize, blockSize);
        block ciphertext;
        ciphertext.insert(ciphertext.end(), recv_buf + i * (clen + 16), recv_buf + (i + 1) * (clen + 16));
        block buffer = AES::Decrypt(key2, ciphertext, clen);
        Bucket bucket = DeserialiseBucket<midNode2>(buffer);

        for (int z = 0; z < Z; z++) {
            Block &block = bucket[z];

            if (block.id != "") { // It isn't a dummy block   
                midNode2* n = convertBlockToNode<midNode2>(block.data);
                if (mid2cache.count(block.id) == 0) {
                    mid2cache.insert(make_pair(block.id, n));
                } else {
                    //delete n;
                }
            }
        }
        i++;
    }
}

void client::FetchleafPath(int leaf)
{
    readCntleaf++;
    // send & receive
    char data[255];
    memset(data, 0, sizeof(data));
    sprintf(data, "%d", leaf);
    data[strlen(data)] = '3';
    data[strlen(data)] = 0;
    send(clientSock, data, strlen(data), 0);
    LOG(INFO) << "[leaf]send and fetch leafpath:" << leaf; 
    //std::cout << "send leafPath:" << leaf << std::endl;
    //Access(bid, node, leaf, leaf);
    unsigned char recv_buf[10240];
    memset(recv_buf, 0, 10240);
    int len = recv(clientSock, recv_buf, sizeof(recv_buf), 0);
    if(len == 1 && recv_buf[0] == '\0') {
        LOG(INFO) << "[leaf]receive NONE.";
        //std::cout << "receive none" << std::endl;
        return;
    }
    recv_buf[len] = '\0';
    LOG(INFO) << "[leaf]receive leaforam bits:" << len;
    //std::cout << "receive leaforam bits:" << len << std::endl;
    int i = 0;
    int blockSize = sizeof(leafNode) * Z;
    int clen = AES::GetCiphertextLength(blockSize);
    while (i * (clen + 16) < len)
    {
        //char*cipher = new char[blockSize];
        //memcpy(cipher, recv_buf + i * blockSize, blockSize);
        block ciphertext;
        ciphertext.insert(ciphertext.end(), recv_buf + i * (clen + 16), recv_buf + (i + 1) * (clen + 16));
        block buffer = AES::Decrypt(key3, ciphertext, clen);
        Bucket bucket = DeserialiseBucket<leafNode>(buffer);

        for (int z = 0; z < Z; z++) {
            Block &block = bucket[z];

            if (block.id != "") { // It isn't a dummy block   
                leafNode* n = convertBlockToNode<leafNode>(block.data);
                if (leafcache.count(block.id) == 0) {
                    leafcache.insert(make_pair(block.id, n));
                } else {
                    //delete n;
                }
            }
        }
        i++;
    }
}

void client::Writemid1Path(int leaf, int d)
{
    int node = GetNodeOnPath(P1, leaf, d);
    if (find(writeviewmap1.begin(), writeviewmap1.end(), node) == writeviewmap1.end()) {

        auto validBlocks = GetIntersectingBlocks1(leaf, d);
        // Write blocks to tree
        Bucket bucket;
        for (int z = 0; z < std::min((int) validBlocks.size(), Z); z++) {
            Block &block = bucket[z];
            block.id = validBlocks[z];
            midNode1* curnode = mid1cache[block.id];
            block.data = convertNodeToBlock<midNode1>(curnode);
            //delete curnode;
            mid1cache.erase(block.id);
        }
        // Fill any empty spaces with dummy blocks
        for (int z = validBlocks.size(); z < Z; z++) {
            Block &block = bucket[z];
            block.id = "";
            block.data.resize(sizeof (midNode1), 0);
        }

        // Write bucket to tree
        writeviewmap1.push_back(node);
        LOG(INFO) << "[mid1]plan to write bucket to store: " << node;
        //std::cout << "Write bucket to store:" << node << std::endl;
        // WriteBucket
        block b = SerialiseBucket(bucket);
        int plaintext_size = sizeof(midNode1) * Z;
        int clen_size = AES::GetCiphertextLength(plaintext_size);
        block ciphertext = AES::Encrypt(key1, b, clen_size, plaintext_size);
        string index = to_string(node);
        int len = index.length();
        ciphertext.insert(ciphertext.end(), index.begin(), index.end());
        ciphertext.push_back((unsigned char)(len));
        ciphertext.push_back('a');
        unsigned char *data = &ciphertext[0];
        send(clientSock, data, ciphertext.size(), 0);
        LOG(INFO) << "[mid1]send ciphertext " << ciphertext.size() << " bits"; 
        char recv_buf[255];
        memset(recv_buf, 0, sizeof(recv_buf));
        recv(clientSock, recv_buf, sizeof(recv_buf), 0);
        if(strcmp(recv_buf, "ACK") == 0) LOG(INFO) << "[mid1]Send Success!!!";
        else LOG(ERROR) << "[mid1]Send WRONG!";
    }
}

void client::Writemid2Path(int leaf, int d)
{
    int node = GetNodeOnPath(P1 * P2, leaf, d);
    if (find(writeviewmap2.begin(), writeviewmap2.end(), node) == writeviewmap2.end()) {

        auto validBlocks = GetIntersectingBlocks2(leaf, d);
        // Write blocks to tree
        Bucket bucket;
        for (int z = 0; z < std::min((int) validBlocks.size(), Z); z++) {
            Block &block = bucket[z];
            block.id = validBlocks[z];
            midNode2* curnode = mid2cache[block.id];
            block.data = convertNodeToBlock<midNode2>(curnode);
            //delete curnode;
            mid2cache.erase(block.id);
        }
        // Fill any empty spaces with dummy blocks
        for (int z = validBlocks.size(); z < Z; z++) {
            Block &block = bucket[z];
            block.id = "";
            block.data.resize(sizeof(midNode2), 0);
        }

        // Write bucket to tree
        writeviewmap2.push_back(node);
        LOG(INFO) << "[mid2]plan to write bucket to store: " << node;
        //std::cout << "Write bucket to store:" << node << std::endl;
        // WriteBucket
        block b = SerialiseBucket(bucket);
        int plaintext_size = sizeof(midNode2) * Z;
        int clen_size = AES::GetCiphertextLength(plaintext_size);
        block ciphertext = AES::Encrypt(key2, b, clen_size, plaintext_size);
        string index = to_string(node);
        int len = index.length();
        ciphertext.insert(ciphertext.end(), index.begin(), index.end());
        ciphertext.push_back((unsigned char)(len));
        ciphertext.push_back('b');
        unsigned char *data = &ciphertext[0];
        send(clientSock, data, ciphertext.size(), 0);
        LOG(INFO) << "[mid2]send ciphertext " << ciphertext.size() << " bits"; 
        char recv_buf[255];
        memset(recv_buf, 0, sizeof(recv_buf));
        recv(clientSock, recv_buf, sizeof(recv_buf), 0);
        if(strcmp(recv_buf, "ACK") == 0) LOG(INFO) << "[mid2]Send Success!!!";
        else LOG(ERROR) << "[mid2]Send WRONG!";
    }
}

void client::WriteleafPath(int leaf, int d)
{
    int node = GetNodeOnPath(P1 * P2 * P2, leaf, d);
    if (find(writeviewmap3.begin(), writeviewmap3.end(), node) == writeviewmap3.end()) {

        auto validBlocks = GetIntersectingBlocks3(leaf, d);
        // Write blocks to tree
        Bucket bucket;
        for (int z = 0; z < std::min((int) validBlocks.size(), Z); z++) {
            Block &block = bucket[z];
            block.id = validBlocks[z];
            leafNode* curnode = leafcache[block.id];
            block.data = convertNodeToBlock<leafNode>(curnode);
            //delete curnode;
            leafcache.erase(block.id);
        }
        // Fill any empty spaces with dummy blocks
        for (int z = validBlocks.size(); z < Z; z++) {
            Block &block = bucket[z];
            block.id = "";
            block.data.resize(sizeof(leafNode), 0);
        }

        // Write bucket to tree
        writeviewmap3.push_back(node);
        LOG(INFO) << "[leaf]plan to write bucket to store: " << node;
        //std::cout << "Write bucket to store:" << node << std::endl;
        // WriteBucket
        block b = SerialiseBucket(bucket);
        int plaintext_size = sizeof(leafNode) * Z;
        int clen_size = AES::GetCiphertextLength(plaintext_size);
        block ciphertext = AES::Encrypt(key3, b, clen_size, plaintext_size);
        string index = to_string(node);
        int len = index.length();
        ciphertext.insert(ciphertext.end(), index.begin(), index.end());
        ciphertext.push_back((unsigned char)(len));
        ciphertext.push_back('c');
        unsigned char *data = &ciphertext[0];
        send(clientSock, data, ciphertext.size(), 0);
        LOG(INFO) << "[leaf]send ciphertext " << ciphertext.size() << " bits"; 
        char recv_buf[255];
        memset(recv_buf, 0, sizeof(recv_buf));
        recv(clientSock, recv_buf, sizeof(recv_buf), 0);
        if(strcmp(recv_buf, "ACK") == 0) LOG(INFO) << "[leaf]Send Success!!!";
        else LOG(ERROR) << "[leaf]Send WRONG!";
    }
}

void client::finalizemid1(bool find)
{
    if (find) {
        for (unsigned int i = readCntmid1; i < 2; i++) {
            int rnd = RandomPath(P1);
            if (std::find(leafList1.begin(), leafList1.end(), rnd) == leafList1.end()) {
                leafList1.push_back(rnd);
            }
            Fetchmid1Path(rnd);
        }
    } else {
        for (int i = readCntmid1; i < 2; i++) {
            int rnd = RandomPath(P1);
            if (std::find(leafList1.begin(), leafList1.end(), rnd) == leafList1.end()) {
                leafList1.push_back(rnd);
            }
            Fetchmid1Path(rnd);
        }
    }
}

void client::finalizemid2(bool find)
{
    if (find) {
        for (unsigned int i = readCntmid2; i < 2; i++) {
            int rnd = RandomPath(P1 * P2);
            if (std::find(leafList2.begin(), leafList2.end(), rnd) == leafList2.end()) {
                leafList2.push_back(rnd);
            }
            Fetchmid2Path(rnd);
        }
    } else {
        for (int i = readCntmid2; i < 2; i++) {
            int rnd = RandomPath(P1 * P2);
            if (std::find(leafList2.begin(), leafList2.end(), rnd) == leafList2.end()) {
                leafList2.push_back(rnd);
            }
            Fetchmid2Path(rnd);
        }
    }
}

void client::finalizeleaf(bool find)
{
    if (find) {
        for (unsigned int i = readCntleaf; i < 2; i++) {
            int rnd = RandomPath(P1 * P2 * P2);
            if (std::find(leafList3.begin(), leafList3.end(), rnd) == leafList3.end()) {
                leafList3.push_back(rnd);
            }
            FetchleafPath(rnd);
        }
    } else {
        for (int i = readCntleaf; i < 2; i++) {
            int rnd = RandomPath(P1 * P2 * P2);
            if (std::find(leafList3.begin(), leafList3.end(), rnd) == leafList3.end()) {
                leafList3.push_back(rnd);
            }
            FetchleafPath(rnd);
        }
    }
}

int client::RandomPath(int maxsize) {
    decltype(dis)::param_type params{0, pow(2, floor(log2(maxsize / Z))) - 1};
    //dis(0, pow(2, floor(log2(maxsize / Z))) - 1);
    dis.param(params);
    int val = dis(mt);
    return val;
}

int client::GetNodeOnPath(int maxSize, int leaf, int curDepth) {
    int depth = floor(log2(maxSize / Z));
    int bucketCount = pow(2, depth + 1) - 1;
    leaf += bucketCount / 2;
    for (int d = depth - 1; d >= curDepth; d--) {
        leaf = (leaf + 1) / 2 - 1;
    }

    return leaf;
}

block client::SerialiseBucket(Bucket bucket) {
    block buffer;

    for (int z = 0; z < Z; z++) {
        Block b = bucket[z];

        // Write block data
        buffer.insert(buffer.end(), b.data.begin(), b.data.end());
    }

    // assert(buffer.size() == Z * (blockSize));

    return buffer;
}

void client::finalize2mid1()
{
    int depth = floor(log2(P1 / Z));
    int cnt = 0;
    for (int d = depth; d >= 0; d--) {
        for (unsigned int i = 0; i < leafList1.size(); i++) {
            cnt++;
            //if (cnt % 1000 == 0 && batchWrite) {
            //    cout << "OMAP:" << cnt << "/" << (depth+1) * leafList.size() << " inserted" << endl;
            //}
            Writemid1Path(leafList1[i], d);
        }
    }

    leafList1.clear();
    mid1modified.clear();
}

void client::finalize2mid2()
{
    int depth = floor(log2(P1 * P2 / Z));
    int cnt = 0;
    for (int d = depth; d >= 0; d--) {
        for (unsigned int i = 0; i < leafList2.size(); i++) {
            cnt++;
            //if (cnt % 1000 == 0 && batchWrite) {
            //    cout << "OMAP:" << cnt << "/" << (depth+1) * leafList.size() << " inserted" << endl;
            //}
            Writemid2Path(leafList2[i], d);
        }
    }

    leafList2.clear();
    mid2modified.clear();
}

void client::finalize2leaf()
{
    int depth = floor(log2(P1 * P2 * P2 / Z));
    int cnt = 0;
    for (int d = depth; d >= 0; d--) {
        for (unsigned int i = 0; i < leafList3.size(); i++) {
            cnt++;
            //if (cnt % 1000 == 0 && batchWrite) {
            //    cout << "OMAP:" << cnt << "/" << (depth+1) * leafList.size() << " inserted" << endl;
            //}
            WriteleafPath(leafList3[i], d);
        }
    }

    leafList3.clear();
    leafmodified.clear();
}

std::vector<string> client::GetIntersectingBlocks1(int x, int curDepth) {
    std::vector<string> validBlocks;

    int node = GetNodeOnPath(P1, x, curDepth);
    for (auto b : mid1cache) {
        string bid = b.first;
        if (b.second != NULL && GetNodeOnPath(P1, b.second->pos, curDepth) == node) {
            validBlocks.push_back(bid);
            if (validBlocks.size() >= Z) {
                return validBlocks;
            }
        }
    }
    return validBlocks;
}

std::vector<string> client::GetIntersectingBlocks2(int x, int curDepth) {
    std::vector<string> validBlocks;

    int node = GetNodeOnPath(P1 * P2, x, curDepth);
    for (auto b : mid2cache) {
        string bid = b.first;
        if (b.second != NULL && GetNodeOnPath(P1 * P2, b.second->pos, curDepth) == node) {
            validBlocks.push_back(bid);
            if (validBlocks.size() >= Z) {
                return validBlocks;
            }
        }
    }
    return validBlocks;
}

std::vector<string> client::GetIntersectingBlocks3(int x, int curDepth) {
    std::vector<string> validBlocks;

    int node = GetNodeOnPath(P1 * P2 * P2, x, curDepth);
    for (auto b : leafcache) {
        string bid = b.first;
        if (b.second != NULL && GetNodeOnPath(P1 * P2 * P2, b.second->pos, curDepth) == node) {
            validBlocks.push_back(bid);
            if (validBlocks.size() >= Z) {
                return validBlocks;
            }
        }
    }
    return validBlocks;
}