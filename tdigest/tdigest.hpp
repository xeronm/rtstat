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

#pragma once

#include <vector>

namespace rtstat
{

class TDigest
{
    public:  
        class WeightedPoint {            
            public:
                WeightedPoint(): value_(0.0), weight_(0.0) {};
                inline void add(double value, double weight) {
                    weight_ += weight;
                    value_ += weight*(value - value_)/weight_; // mean or mass center
                };

                inline void set(double value, double weight) {
                    weight_ = weight;
                    value_ = value;
                };
                inline void set(WeightedPoint source) {
                    weight_ = source.weight_;
                    value_ = source.value_;
                };

                inline double value() const { return value_; };
                inline double weight() const { return weight_; };
            private:
                double value_;
                double weight_;
        };

        explicit TDigest(size_t delta = 100, size_t excessiveGrowthPCT = 150)
            // excessive growth factor in hundreds - maxSize = delta + delta*excessiveGrowth/100
            : delta_(delta), excessiveGrowthPCT_(excessiveGrowthPCT), min_(0.0), max_(0.0),
            centroidCount_(0), totalWeight_(0.0), centroids_(delta + delta*excessiveGrowthPCT/100 + 2) {};

        size_t merge(TDigest digest);
        // merging sorted values into T-digest, return count of merged values
        size_t merge(std::vector<double>::iterator begin, std::vector<double>::iterator end);

        void shrink(); // shrink T-digest to target compress factor

        void add(std::vector<WeightedPoint> values); // add unsorted observation values into T-digest using clustering algorythm
        void add(double value); // add single observation value into T-digest using clustering algorythm

        double quantile(double q) const;
        void describe(FILE * f) const;
    private:
        inline void clusteringAdd(double value, double weight);
        double weightLeft(size_t index) const; // Wleft from T-Digest paper

        std::vector<WeightedPoint> centroids_;
        double min_;
        double max_;
        size_t centroidCount_; // initialized centorids count
        double totalWeight_; // total weight or observations count N from T-Digest paper
        size_t delta_; // compress factor
        size_t excessiveGrowthPCT_; // excessive growth factor in hundreds - maxSize = delta_*excessiveGrowth_/100
};

}