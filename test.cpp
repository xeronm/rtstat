/*

Copyright (c) 2019 <copyright holders>

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.

*/

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <random>

#include "p2/p2.hpp"

int main (int argc, char *argv[])
{
    fprintf(stdout, "Hello World!!!\n");

    double Q[] = {0.25, 0.5, 0.75, 0.95, 0.99, 0.999, 0.9999};
    std::vector<double> quantiles(Q, Q + sizeof(Q) / sizeof(double));

    rtstat::P2 p2(quantiles);

    std::default_random_engine generator;
    std::normal_distribution<double> distribution(50.0, 5.0);

    for (int i=0; i<=20; ++i) {
        p2.add(distribution(generator));
    } 

    return 0;
}