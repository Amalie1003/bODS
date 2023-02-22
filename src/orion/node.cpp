#include "node.hpp"

int C, P1, P2;

int RandomPos(int maxsize)
{
	std::random_device rd;
	std::mt19937 mt(rd());
	std::uniform_int_distribution<int> dis(0,pow(2, floor(log2(maxsize / Z))) - 1);
	int val = dis(mt);
	return val;
}


bool kppair::operator<(const kppair& b)const
{
	return key < b.key;
}

bool kppair::operator<=(const kppair& b)const
{
	return key <= b.key;
}

bool kppair::operator>(const kppair& b)const
{
	return key > b.key;
}

bool kppair::operator>=(const kppair& b)const
{
	return key >= b.key;
}

bool kppair::operator==(const kppair rhs)const
{
	return key == rhs.key;
}


leafNode::leafNode()
{
	c = 0; 
	pos = RandomPos(P1*P2*P2);
}

leafNode::leafNode(string k, int v)
{
	kvpair* kv = new kvpair(k,v);
	arr.push_back(kv);
	c = 1;
	max_value = k;
	pos = RandomPos(P1 * P2 * P2);
}

leafNode* leafNode::Insert(string key, int value){
    kvpair* insert_kvpair = new kvpair(key, value);
    if(this->c < C)
    {
        auto it = arr.begin();
        for(; it < arr.end(); it++)
        {
            int i = 0;
            if(arr[i]->key == key){
                arr[i]->value = value;
                return NULL;
            }
            if(arr[i]->key > key) break;
            i++;
        }
        arr.emplace(it, insert_kvpair);
        c++;
        max_value = arr[c-1]->key;
        return NULL;
    }
    else{
        leafNode *newNode = new leafNode();
        std::vector<kvpair*> copy;
        auto it = arr.begin();
        for(unsigned int i = 0; i < arr.size(); i++)
        {
			if(arr[i]->key == key){
				arr[i]->value = value;
				return NULL;
			}
            if(arr[i]->key > key) break;
            it++; 
        }
        arr.emplace(it, insert_kvpair);
        for(int i = 0; i <= c; i++){
            if(i < (c + 1)/2) copy.push_back(arr[i]);
            else newNode->arr.push_back(arr[i]);
        }
        std::vector<kvpair*>().swap(arr);
        arr.assign(copy.begin(), copy.end());

        newNode->c = c + 1 - copy.size();
        newNode->max_value = newNode->arr[newNode->c - 1]->key;
        this->c = copy.size();
        this->max_value = arr[copy.size() - 1]->key;
        return newNode;
    }
}

kvpair* leafNode::Search(string key)
{
    for(int i = 0; i < c; i++)
    {
        if(arr[i]->key == key)
            return arr[i]; 
    }
    return NULL;
}

midNode2::midNode2()
{
	p2 = 0; 
	pos = RandomPos(P1*P2); 
	childMap.resize(P2);
}

midNode2* midNode2::Insert(leafNode* leaf)
{
	kppair *kp = new kppair(leaf->max_value, leaf->pos);
    if (this->p2 < P2)
	{
		this->childMap[p2] = kp;
		//this->my_map[leaf->max_value] = leaf->pos;
		this->p2++;
		sort(childMap.begin(), childMap.begin() + p2);
		leaf->old_max = leaf->max_value;
		//this->old_max = this->max_value;
		this->max_value = childMap[p2 - 1]->key;

		for(std::vector<kppair*>::iterator iter = childMap.begin(); iter != childMap.end(); iter++)
		{
			if(*iter != NULL)
				std::cout << (*iter)->key << " " << (*iter)->pos << std::endl;
		}

		return NULL;
	}
	else
	{
		midNode2* newNode = new midNode2();
		childMap.push_back(kp);
		p2++;
		sort(childMap.begin(), childMap.end());
		leaf->old_max = leaf->max_value;
		//this->old_max = this->max_value;
		for (int i = 0; i < P2 + 1 - (P2 + 1) / 2; i++)
		{
			int index = i + (P2 + 1) / 2;
			newNode->childMap[i] = childMap[index];
			childMap[index] = NULL;
			p2--;
			newNode->p2++;			
		}
		childMap.resize(P2);
		max_value = childMap[p2 - 1]->key;
		newNode->max_value = newNode->childMap[newNode->p2 - 1]->key; 
		//std::map<string, int>().swap(this->my_map);
		//malloc_trim(0);
		//this->my_map = copy;
		return newNode;
	}
}

void midNode2::Update(leafNode *l)
{
	int num = childMap.size();	
    //std::map<string, int>::iterator it = this->my_map.begin();
	//std::cout<<my_map.begin()->second<<std::endl;
	for(std::vector<kppair*>::iterator iter = childMap.begin(); iter != childMap.end(); iter++)
	{
		if(*iter != NULL)
			std::cout << (*iter)->key << " " << (*iter)->pos << std::endl;
	}
		/*
	while (it != this->my_map.end())
	{
		//if (it->first == l->old_max)
		{
			std::cout << "hi";
			//this->my_map.erase(it->first);
			//break;
		}
		it++;
	}	*/
	int i = 0;
	for(; i < p2; i++)
	{
		if(childMap[i]->key == l->old_max)
		{
			childMap[i]->key = l->max_value;
			break;
		}
	}
	if(l->max_value != l->old_max) sort(childMap.begin(), childMap.begin() + p2);
	l->old_max = l->max_value;
	//this->old_max = this->max_value;
	this->max_value = childMap[p2 - 1]->key;
	return;
}

int midNode2::Search(string key, string& l_key)
{
	for(int i = 0; i < p2; i++)
	{
		if(childMap[i]->key >= key)
		{
			l_key = childMap[i]->key;
			return childMap[i]->pos;
		}
	}
	l_key = max_value;
	return childMap[p2 - 1]->pos;
}

midNode1::midNode1()
{
	p2 = 0; 
	pos = RandomPos(P1); 
	childMap.resize(P2);
}

midNode1* midNode1::Insert(midNode2* mid_node2)
{
	kppair *kp = new kppair(mid_node2->max_value, mid_node2->pos);
	if (this->p2 < P2)
	{
		childMap[p2] = kp;
		this->p2++;
		sort(childMap.begin(), childMap.begin() + p2);
		mid_node2->old_max = mid_node2->max_value;
		//this->old_max = this->max_value;
		this->max_value = childMap[p2 - 1]->key;
		return NULL;
	}
	else
	{
		midNode1* newNode = new midNode1();
		childMap.push_back(kp);
		p2++;
		sort(childMap.begin(), childMap.end());
		mid_node2->old_max = mid_node2->max_value;
		//this->old_max = this->max_value;
		for (int i = 0; i < P2 + 1 - (P2 + 1) / 2; i++)
		{
			int index = i + (P2 + 1) / 2;
			newNode->childMap[i] = childMap[index];
			childMap[index] = NULL;
			p2--;
			newNode->p2++;
		}
		childMap.resize(P2);
		max_value = childMap[p2 - 1]->key;
		newNode->max_value = newNode->childMap[newNode->p2 - 1]->key;
		return newNode;
	}
}

void midNode1::Update(midNode2 *m2)
{
	int num = childMap.size();
	int i = 0;
	for(; i < p2; i++)
	{
		if(childMap[i]->key == m2->old_max)
		{
			childMap[i]->key = m2->max_value;
			break;
		}
	}
	if(m2->max_value != m2->old_max) sort(childMap.begin(), childMap.begin() + p2);
	m2->old_max = m2->max_value;
	//this->old_max = this->max_value;
	this->max_value = childMap[p2 - 1]->key;
	return;
}

int midNode1::Search(string key, string& m2_key)
{
	for(int i = 0; i < p2; i++)
	{
		if(childMap[i]->key >= key)
		{
			m2_key = childMap[i]->key;
			return childMap[i]->pos;
		}
	}
	m2_key = max_value;
	return childMap[p2 - 1]->pos;
}

rootNode::rootNode()
{
	p1 = 0; 
	childMap.resize(P1);
}

void rootNode::Update(midNode1* m1)
{
	int num = childMap.size();
	int i = 0;
	for(; i < p1; i++)
	{
		if(childMap[i]->key == m1->old_max)
		{
			childMap[i]->key = m1->max_value;
			break;
		}
	}
	if(m1->max_value != m1->old_max) sort(childMap.begin(), childMap.begin() + p1);
	m1->old_max = m1->max_value;
	//this->old_max = this->max_value;
	this->max_value = childMap[p1 - 1]->key;
	return;
}

bool rootNode::Insert(midNode1* m1)
{
	kppair *kp = new kppair(m1->max_value, m1->pos);
	if (this->p1 < P1)
	{
		childMap[p1] = kp;
		this->p1++;
		sort(childMap.begin(), childMap.begin() + p1);
		this->max_value = childMap[p1 -1]->key;
		m1->old_max = m1->max_value;
		return true;
	}
	else return false;
}

int rootNode::Search(string key, string& m1_key)
{
	for(int i = 0; i < p1; i++)
	{
		if(childMap[i]->key >= key)
		{
			m1_key = childMap[i]->key;
			return childMap[i]->pos;
		}
	}
	m1_key = max_value;
	return childMap[p1 - 1]->pos;
}

