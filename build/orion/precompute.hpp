#ifndef _PRECOMPUTE_H_
#define _PRECOMPUTE_H_

#include <vector>
#include <cmath>
#include <limits.h>
#include <iostream>

float blockToleaf(int n);
float bitToKB(int n);
float C1(int p1);
float C2(int p1, int p2);
float C3(int p1, int p2, int c);
float padfun(int p1, int p2, int c);
std::vector<int> param_bODS(int N);

#endif
