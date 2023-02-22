#include "ORAM.hpp"
#include "../utils/Utilities.h"
#include <algorithm>
#include <iomanip>
#include <fstream>
#include <random>
#include <cmath>
#include <cassert>
#include <cstring>
#include <map>
#include <stdexcept>

template<class Node>
ORAM<Node>::ORAM(int maxSize, bytes<Key> k)
{
    key = k;
    AES::Setup();
    depth = floor(log2(maxSize / Z));
    bucketCount = pow(2, depth + 1) - 1;
    blockSize = sizeof (Node); // B
    size_t blockCount = Z * (pow(2, depth + 1) - 1);
    size_t storeBlockSize = IV + AES::GetCiphertextLength(Z * (blockSize));
    size_t storeBlockCount = blockCount;
    clen_size = AES::GetCiphertextLength((blockSize) * Z);
    plaintext_size = (blockSize) * Z;
    store = new RAMStore(storeBlockCount, storeBlockSize);
    for (size_t i = 0; i < bucketCount; i++) {
        Bucket bucket;
        for (int z = 0; z < Z; z++) {
            bucket[z].id = "";
            bucket[z].data.resize(blockSize, 0);
        }
        //WriteBucket(i, bucket);
        block b = SerialiseBucket(bucket);
        block ciphertext = AES::Encrypt(key, b, clen_size, plaintext_size);
        block buffer = AES::Decrypt(key, ciphertext, clen_size);
        store->Write(i, ciphertext);
    }
}
template<class Node>
ORAM<Node>::~ORAM() {
}

template<class Node>
void ORAM<Node>::WriteBucket(int index, block ciphertext) {
    // block b = SerialiseBucket(bucket);
    // block ciphertext = AES::Encrypt(key, b, clen_size, plaintext_size);
    store->Write(index, ciphertext);
    // TO DO1: decrypt

}

template <class Node>
int ORAM<Node>::GetNodeOnPath(int leaf, int curDepth) {
    //int depth = floor(log2(maxSize / Z));
    //int bucketCount = pow(2, depth + 1) - 1;
    leaf += bucketCount / 2;
    for (int d = depth - 1; d >= curDepth; d--) {
        leaf = (leaf + 1) / 2 - 1;
    }

    return leaf;
}

// Fetches blocks along a path, adding them to the cache(拆分,发送到client-cache)
template<class Node>
void ORAM<Node>::FetchPath(int leaf) {
    readCnt++;
    block result;
    for (size_t d = 0; d <= depth; d++) {
        int node = GetNodeOnPath(leaf, d);

        if (find(readviewmap.begin(), readviewmap.end(), node) != readviewmap.end()) {
            continue;
        } else {
            readviewmap.push_back(node);
        }

        block ciphertext = store->Read(node);
        block buffer = AES::Decrypt(key, ciphertext, clen_size);
        //LOG(INFO) << "偷偷解密的:" << &buffer[0];
        result.insert(result.end(), ciphertext.begin(), ciphertext.end());
    }
    // 写回
    int send_len = result.size();
    if(send_len == 0)
    {
        send_len = 1;
        result.push_back('\0');
        std::cout << "Send " << send_len << " bits" << std::endl;
    }
    unsigned char *data = &result[0];
    send(conn, data, send_len, 0);
    LOG(INFO) << "Send " << send_len << " bits";
    //Bucket bucket = ReadBucket(node);
}

template<class Node>
void ORAM<Node>::start() {
    readviewmap.clear();
    readCnt = 0;
}

template <class Node>
block ORAM<Node>::SerialiseBucket(Bucket bucket) {
    block buffer;

    for (int z = 0; z < Z; z++) {
        Block b = bucket[z];

        // Write block data
        buffer.insert(buffer.end(), b.data.begin(), b.data.end());
    }

    // assert(buffer.size() == Z * (blockSize));

    return buffer;
}