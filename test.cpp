/*

Copyright (c) 2019 Denis Muratov <xeronm@gmail.com>

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
#include "tdigest/tdigest.hpp"

#define SAMLPE_PASS_COUNT 10

class PerfReportItem {
    public:
        PerfReportItem(const char* distribution, const char* algorythm, size_t samples, double RMSE, double time_stat)
            : RMSE_(RMSE), samples_(samples), distribution_(distribution), algorythm_(algorythm), time_stat_(time_stat) {};

        double RMSE_;
        size_t samples_;
        std::string distribution_;
        std::string algorythm_;
        double time_stat_;
};

void run_perf_test_p2(std::vector<double> set, std::vector<double> quantiles, double* msre, double* time_stat) 
{
    rtstat::P2 p2(quantiles);

    printf("== P2 =================\n");
    auto start = std::chrono::high_resolution_clock::now();
    for (std::vector<double>::iterator it=set.begin(); it!=set.end(); ++it) {
        p2.add(*it);
    } 
    auto end = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> diff = end - start;
    std::chrono::duration<double, std::nano> per_ns = (end - start)/set.size();
    *time_stat += per_ns.count();
    printf("time spent (sec): %f, peritem (ns): %0.2f\n", diff, per_ns);

    //p2.describe(stdout);
    printf("=============\n");

    std::vector<double> sset(set);
    std::sort(sset.begin(), sset.end());
    double mse = 0;
    printf("   quantile          O        P^2\n");
    for (auto it=quantiles.begin(); it!=quantiles.end(); ++it) {
        double qp = p2.quantile(it - quantiles.begin());
        double qo = sset[(size_t) (sset.size()**it)];
        mse += (qp -  qo)*(qp -  qo);
        printf(" %10.4f %10.4f %10.4f\n", *it, qo, qp );
    } 

    *msre = mse/quantiles.size();
    printf("RMSE: %f\n", *msre);
    printf("== P2 =================\n\n");
}

void run_perf_test_tdigest(std::vector<double> set, std::vector<double> quantiles, double* msre, double* time_stat) 
{
    rtstat::TDigest td(quantiles.size()*5, 200);

    printf("== T-digest ===========\n");

    auto start = std::chrono::high_resolution_clock::now();
    for (auto it=set.begin(); it!=set.end(); ++it) {
        td.add(*it);
    } 
    auto end = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> diff = end - start;
    std::chrono::duration<double, std::nano> per_ns = (end - start)/set.size();
    *time_stat += per_ns.count();
    printf("time spent (sec): %f, peritem (ns): %0.2f\n", diff, per_ns);

    //td.describe(stdout);
    printf("=============\n");

    std::vector<double> sset(set);
    std::sort(sset.begin(), sset.end());
    double mse = 0;
    printf("   quantile          O   T-digest\n");
    for (auto it=quantiles.begin(); it!=quantiles.end(); ++it) {
        double qp = td.quantile(*it);
        double qo = sset[(size_t) (sset.size()**it)];
        mse += (qp -  qo)*(qp -  qo);
        printf(" %10.4f %10.4f %10.4f\n", *it, qo, qp );
    } 

    *msre = mse/quantiles.size();
    printf("RMSE: %f\n", *msre);

    printf("== T-digest ===========\n\n");
}

void run_perf_test(std::vector<PerfReportItem>& report, size_t samples, std::vector<double> quantiles) 
{
    std::default_random_engine generator(1);
    std::normal_distribution<double> norm(60.0, 10.0);
    std::lognormal_distribution<double> lognorm(0.0, 0.3);

    std::vector<double> sample_n(samples);
    std::generate(sample_n.begin(), sample_n.end(), [&norm, &generator]() { return norm(generator); } );
    std::vector<double> sample_ln(samples);
    std::generate(sample_ln.begin(), sample_ln.end(), [&lognorm, &generator]() { return lognorm(generator)*10+50; } );

    double rmse;
    double time_stat;
    printf("\n\n=============\n");
    printf("Distribution: Normal\nSamples: %d\n", samples);
    rmse = 0;
    time_stat = 0;
    for (size_t i=0; i<SAMLPE_PASS_COUNT; ++i) {
        run_perf_test_p2(sample_n, quantiles, &rmse, &time_stat);
    }
    report.push_back(PerfReportItem("Normal", "P^2", samples, rmse, time_stat/SAMLPE_PASS_COUNT));

    rmse = 0;
    time_stat = 0;
    for (size_t i=0; i<SAMLPE_PASS_COUNT; ++i) {
        run_perf_test_tdigest(sample_n, quantiles, &rmse, &time_stat);
    }
    report.push_back(PerfReportItem("Normal", "T-digest", samples, rmse, time_stat/SAMLPE_PASS_COUNT));

    printf("=============\n");
    printf("Distribution: Log-normal\nSamples: %d\n", samples);
    rmse = 0;
    time_stat = 0;
    for (size_t i=0; i<SAMLPE_PASS_COUNT; ++i) {
        run_perf_test_p2(sample_ln, quantiles, &rmse, &time_stat);
    }
    report.push_back(PerfReportItem("Log-normal", "P^2", samples, rmse, time_stat/SAMLPE_PASS_COUNT));

    rmse = 0;
    time_stat = 0;
    for (size_t i=0; i<SAMLPE_PASS_COUNT; ++i) {
        run_perf_test_tdigest(sample_ln, quantiles, &rmse, &time_stat);
    }
    report.push_back(PerfReportItem("Log-normal", "T-digest", samples, rmse, time_stat/SAMLPE_PASS_COUNT));
}

int main (int argc, char *argv[])
{
    double Q[] = {0.25, 0.5, 0.75, 0.95};
    std::vector<double> quantiles(Q, Q + sizeof(Q) / sizeof(double));    

    double Q2[] = {0.25, 0.5, 0.75, 0.95, 0.99};
    std::vector<double> quantiles2(Q2, Q2 + sizeof(Q2) / sizeof(double));    

    double Q3[] = {0.25, 0.5, 0.75, 0.95, 0.99, 0.999};
    std::vector<double> quantiles3(Q3, Q3 + sizeof(Q3) / sizeof(double));    

    std::default_random_engine generator(1);
    std::normal_distribution<double> norm(60.0, 10.0);
    std::lognormal_distribution<double> lognorm(0.0, 1.0);

    std::vector<PerfReportItem> report;

    run_perf_test(report, 100, quantiles);
    run_perf_test(report, 1000, quantiles2);
    run_perf_test(report, 10000, quantiles3);
    run_perf_test(report, 50000, quantiles3);
    run_perf_test(report, 100000, quantiles3);

    printf("Final report: %d\n", report.size());
    printf(" distribution       algo    samples       rmse   item(ns)\n");
    for (auto it=report.begin(); it!=report.end(); ++it) {
        printf(" %12s %10s %10d %10.4f %10.2f\n", it->distribution_.c_str(), it->algorythm_.c_str(), it->samples_, it->RMSE_, it->time_stat_);
    }

    printf("Done.\n");

    return 0;
}