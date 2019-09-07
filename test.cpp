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
#include <chrono>
#include <algorithm>

#include "p2/p2.hpp"

void run_perf_test_p2(std::vector<double> set, std::vector<double> quantiles) 
{
    rtstat::P2 p2(quantiles);

    auto start = std::chrono::high_resolution_clock::now();
    for (std::vector<double>::iterator it=set.begin(); it!=set.end(); ++it) {
        p2.add(*it);
    } 
    auto end = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> diff = end - start;
    printf("time spent (sec): %f\n", diff);

    p2.describe(stdout);
    printf("=============\n");

    std::vector<double> sset(set);
    std::sort(sset.begin(), sset.end());
    double mse = 0;
    printf("   quantile          O        P^2\n");
    for (std::vector<double>::iterator it=quantiles.begin(); it!=quantiles.end(); ++it) {
        double qp = p2.quantile(it - quantiles.begin());
        double qo = sset[(size_t) (sset.size()**it)];
        mse += (qp -  qo)*(qp -  qo);
        printf(" %10.4f %10.4f %10.4f\n", *it, qo, qp );
    } 

    printf("RMSE: %f\n", sqrt(mse/quantiles.size()));
}

void run_perf_test(size_t samples, std::vector<double> quantiles) 
{
    std::default_random_engine generator(1);
    std::normal_distribution<double> norm(60.0, 10.0);
    std::lognormal_distribution<double> lognorm(0.0, 0.3);

    std::vector<double> sample_n(samples);
    std::generate(sample_n.begin(), sample_n.end(), [&norm, &generator]() { return norm(generator); } );
    std::vector<double> sample_ln(samples);
    std::generate(sample_ln.begin(), sample_ln.end(), [&lognorm, &generator]() { return lognorm(generator)*10+50; } );

    printf("\n\n=============\n");
    printf("Normal distribution: %d samples\n", samples);
    run_perf_test_p2(sample_n, quantiles);
    printf("=============\n");
    printf("Log-normal distribution: %d samples\n", samples);
    run_perf_test_p2(sample_ln, quantiles);
}

int main (int argc, char *argv[])
{
    printf("Hello World!!!\n");

    double Q[] = {0.25, 0.5, 0.75, 0.95, 0.99, 0.999
        //, 0.9999
    };
    std::vector<double> quantiles(Q, Q + sizeof(Q) / sizeof(double));    

    std::default_random_engine generator(1);
    std::normal_distribution<double> norm(60.0, 10.0);
    std::lognormal_distribution<double> lognorm(0.0, 1.0);

    run_perf_test(100, quantiles);
    run_perf_test(1000, quantiles);
    run_perf_test(10000, quantiles);
    run_perf_test(50000, quantiles);

    return 0;
}