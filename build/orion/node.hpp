#pragma once
#ifndef _NODE_H_
#define _NODE_H_
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
#include <malloc.h>
using namespace std;

extern int C, P1, P2;

int RandomPos(int maxsize);

class kvpair
{
public:
    string key;
    int value;
    kvpair(string k, int v){key = k; value = v;}
    ~kvpair(){}
};

class kppair
{
public:
    string key;
    int pos;
    kppair(){key = ""; pos = 0;}
    kppair(string k, int p){key = k; pos = p;}
    kppair(kppair*kp){key = kp->key; pos = kp->pos;}
    ~kppair(){}

    bool operator<(const kppair& b) const ;
    bool operator>(const kppair& b) const ;
    bool operator<=(const kppair& b) const ;
    bool operator>=(const kppair& b) const ;   
    bool operator==(const kppair rhs)const ; 
    friend ostream& operator<<(ostream &o, kppair& id);
};

class leafNode
{
//private:
    /* data */
    
public:
    leafNode();
    leafNode(string k, int v);
    ~leafNode(){}
    leafNode* Insert(string key, int value);
    kvpair* Search(string key);
    std::vector<kvpair*> arr;
    int c;
    string old_max; //old max_value to index the element in its parent node
    string max_value; 
    int pos;
    std::vector<kppair*> childMap;
};

class midNode2
{
private:
    /* data */
    
public:
    midNode2();
    ~midNode2(){};
    midNode2* Insert(leafNode* l);
    void Update(leafNode *l);
    //return pos of leafNode
    int Search(string key, string& l_key);
    int p2;
    string old_max;
    string max_value;
    std::vector<kppair*> childMap;
    int pos;
};

class midNode1
{
private:
    /* data */
public:
    midNode1();
    ~midNode1(){};
    midNode1* Insert(midNode2* m2);
    void Update(midNode2* m2);
    int Search(string key, string& m2_key);
    int p2;
    string old_max;
    string max_value;
    std::vector<kppair*> childMap;
    int pos;
};

class rootNode
{
public:
    int p1;
    string max_value;
    std::vector<kppair*> childMap;
    rootNode();
    ~rootNode(){}
    void Update(midNode1* m1);
    bool Insert(midNode1* m1);
    int Search(string key, string& m1_key);
};

#endif
