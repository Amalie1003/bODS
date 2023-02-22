#include "server.hpp"
server::server()
{
    bytes<Key> key1{0};
	bytes<Key> key2{1};
    bytes<Key> key3{2};
    oram1 = new ORAM<midNode1>(P1, key1);
    oram2 = new ORAM<midNode2>(P1 * P2, key2);
    oram3 = new ORAM<leafNode>(P1 * P2 * P2, key3);
}

server::~server()
{
}

void server::init()
{
    listenfd = socket(AF_INET, SOCK_STREAM, 0);
    if (listenfd == -1) {
        LOG(ERROR) << "ERROR socket";
        //std::cout << "Error: socket" << std::endl;
        return;
    }
    // bind
    addr.sin_family = AF_INET;
    addr.sin_port = htons(8000);
    addr.sin_addr.s_addr = INADDR_ANY;
    if (bind(listenfd, (struct sockaddr*)&addr, sizeof(addr)) == -1) {
        LOG(ERROR) << "ERROR bind";
        //std::cout << "Error: bind" << std::endl;
        return;
    }
    // listen
    if(listen(listenfd, 5) == -1) {
        LOG(ERROR) << "ERROR listen";
        //std::cout << "Error: listen" << std::endl;
        return;
    }
    // accept
    char clientIP[INET_ADDRSTRLEN] = "";
    socklen_t clientAddrLen = sizeof(clientAddr);
    LOG(INFO) << "...listening...";
    //std::cout << "...listening" << std::endl;
    conn = accept(listenfd, (struct sockaddr*)&clientAddr, &clientAddrLen);
    if (conn < 0) {
        LOG(ERROR) << "ERROR accept";
        //std::cout << "Error: accept" << std::endl;
        return;
    }
    inet_ntop(AF_INET, &clientAddr.sin_addr, clientIP, INET_ADDRSTRLEN);
    LOG(INFO) << "...connect " << clientIP << ":" << ntohs(clientAddr.sin_port);
    //std::cout << "...connect " << clientIP << ":" << ntohs(clientAddr.sin_port) << std::endl;

    oram1->conn = conn;
    oram2->conn = conn;
    oram3->conn = conn;
    char buf[102400];

    int recv_count1 = 0;
    int recv_count2 = 0;
    int recv_count3 = 0;

    while (true)
    {
        memset(buf, 0, sizeof(buf));
        int len = recv(conn, buf, sizeof(buf), 0);
	    if (len < 0) LOG(ERROR) << "receive Failed";
        LOG(INFO) << "[server] receives " << len << " bits:";
        //std::cout << "receive " << len << " bits" << std::endl;
        buf[len] = '\0';
        if (recv_count1 % 2 == 0) oram1->start();
        if (recv_count2 % 2 == 0) oram2->start();
        if (recv_count3 % 2 == 0) oram3->start();
        if (buf[len - 1] <= '3' && buf[len - 1] >= '1')
        {
            int num = atoi(buf);
            int leaf = num / 10;
            if(buf[len - 1] == '1') 
            {
                recv_count1++;
                LOG(INFO) << "Begin fetch oram1 path:" << leaf;
                oram1->FetchPath(leaf);
                LOG(INFO) << "Finish fetch oram1 path:" << leaf;
                //std::cout << "Fetch Oram1 Path " << leaf << std::endl;
            }
            if(buf[len - 1] == '2') 
            {
                recv_count2++;
                LOG(INFO) << "Begin fetch oram2 path:" << leaf;
                oram2->FetchPath(leaf);
                LOG(INFO) << "Finish fetch oram2 path:" << leaf;
            }
            if(buf[len - 1] == '3') 
            {
                recv_count3++;
                LOG(INFO) << "Begin fetch leaforam path:" << leaf;
                oram3->FetchPath(leaf);
                LOG(INFO) << "Finish fetch leaforam path:" << leaf;
            }
        }
        if (buf[len - 1] <= 'c' && buf[len - 1] >= 'a')
        {
            int index_len = buf[len - 2];
            char *ind = new char(len);
            memcpy(ind, buf + (len - index_len - 2), index_len);
            int index = atoi(ind);
            block ciphertext;
            ciphertext.insert(ciphertext.end(), buf, buf + (len - index_len - 2));

            if(buf[len - 1] == 'a') 
            {
                LOG(INFO) << "Begin Write to oram1-RAMstore : index = " << index;
                oram1->WriteBucket(index, ciphertext);
                LOG(INFO) << "Finish Write to oram1-RAMstore : index = " << index;
            }
            if(buf[len - 1] == 'b') 
            {
                LOG(INFO) << "Begin Write to oram2-RAMstore : index = " << index;
                oram2->WriteBucket(index, ciphertext);
                LOG(INFO) << "Finish Write to oram2-RAMstore : index = " << index;
                //std::cout << "Write to Oram2 Path " << index << std::endl;
            }
            if(buf[len - 1] == 'c') 
            {
                LOG(INFO) << "Begin Write to leaforam-RAMstore : index = " << index;
                oram3->WriteBucket(index, ciphertext);
                LOG(INFO) << "Finish Write to leaforam-RAMstore : index = " << index;
                //std::cout << "Write to leafOram Path " << index << std::endl;
            }
            char send_buf[255];
            memset(send_buf, 0, sizeof(send_buf));
            strcpy(send_buf, "ACK");
            send(conn, send_buf, sizeof(send_buf), 0);
            LOG(INFO) << "Send ACK!";
        }
        if (strcmp(buf, "exit") == 0) {
            std::cout << "...disconnect " << clientIP << ":" << ntohs(clientAddr.sin_port) << std::endl;
            break;
        }
        std::cout << std::endl;
    }
    
}
