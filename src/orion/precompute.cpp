#include "precompute.hpp"
#include "glog/logging.h"

float blockToleaf(int n)
{
    float temp = log2(ceil((float)n / 4));
    return ceil(temp);
}
float bitToKB(int n)
{
    return (float)n / 8 / 1024;
}
float C1(int p1)
{
    float temp = blockToleaf(p1);
    return p1 * (temp + 32) * 2;
}
float C2(int p1, int p2)
{
    float temp1 = blockToleaf(p1);
    float temp2 = blockToleaf(p1 * p2);
    float temp3 = blockToleaf(p1 * p2 * p2);
    float temp4 = temp1 * (32 + temp2) + temp2 * (32 + temp3);
    return 4 * p2 * temp4;
}
float C3(int p1, int p2, int c)
{
    float temp1 = blockToleaf(p1 * p2 * p2);
    return 256 * c * temp1 * 3;
}
float padfun(int p1, int p2, int c)
{
    float ret = C1(p1) + C2(p1, p2) + C3(p1, p2, c);
    return ret;
}
std::vector<int> param_bODS(int N)
{
    std::vector<int> ret(4);
    int c = 2 * int(pow(N, 0.3));
    int p2_max = 2 * int(pow(N, 0.3));
    float communication = INT_MAX;
    while (c > 0)
    {
        for (int p2 = 1; p2 <= p2_max; p2++)
        {
            int p1 = int(8*N/(p2*p2*c));
            if(p1 <= 0) p1 = 1;
            float temp = padfun(p1, p2, c);
            if(temp < communication)
            {
                communication = temp;
                ret[0]=p1;
                ret[1]=p2;
                ret[2]=c;
                ret[3]=communication;
            }
        }
	c--;
        
    }
    LOG(INFO) << "P1=" << ret[0] << ",P2=" <<ret[1] << ",C=" << ret[2];
    //std::cout << ret[0] << " " << ret[1] << " " << ret[2] << std::endl;
    return ret;
}