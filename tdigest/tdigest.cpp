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
#include <algorithm>
#include <vector>
#include <math.h>

#include "tdigest.hpp"

namespace rtstat
{

// Scaling function from folly TDigest
static double scalingK(double q, double d) {
    if (q >= 0.5) {
        return d - d * sqrt(0.5 - 0.5 * q);
    }
    return d * sqrt(0.5 * q);
}

// Inverse scaling function from folly TDigest
static double scalingKInverse(double k, double d) {
    double k_div_d = k / d;
    if (k_div_d >= 0.5) {
        double base = 1 - k_div_d;
        return 1 - 2 * base * base;
    } else {
        return 2 * k_div_d * k_div_d;
    }
}

void TDigest::describe(FILE * f) const
{
    fprintf(f, "\ncentroids: %d, min:%f, max:%f\n       value     weight\n", centroidCount_, min_, max_);
    for (size_t i=0; i<centroidCount_; ++i) {
        fprintf(f, " %10.4f %10.4f\n", centroids_[i].value(), centroids_[i].weight());
    }
}

void TDigest::add(double value) {
    clusteringAdd(value, 1);
}

void TDigest::add(std::vector<TDigest::WeightedPoint> values) 
{
    for (std::vector<TDigest::WeightedPoint>::iterator it=values.begin(); it!=values.end(); ++it) {
        clusteringAdd(it->value(), it->weight());
    }
};

double TDigest::weightLeft(size_t index) const
{    
    if (index > centroidCount_/2) {
        // backward cummulative sum
        double weight = totalWeight_;
        for (size_t i=centroidCount_ - 1; i>=index; --i) {
            weight -= centroids_[i].weight();
        }
        return weight;
    }
    else {
        // forward cummulative sum
        double weight = 0;
        for (size_t i=0; i < index; ++i) {
            weight += centroids_[i].weight();
        }
        return weight;
    }
}

inline void TDigest::clusteringAdd(double value, double weight) 
{
    if (centroidCount_ == 0) {
        min_ = value;
        max_ = value;
        totalWeight_ = weight;
        centroids_[0].set(value, weight);
        ++centroidCount_;

        return;
    }

    if (min_ > value) { min_ = value; }
    if (max_ < value) { max_ = value; }

    // Find centroids with minimum distance to xn
    //      "To add a new value xn with a weight wn, the set of centroids is found that have minimum distance to xn."
    auto Z = std::upper_bound (centroids_.begin(), centroids_.begin() + centroidCount_, value,
            [](const double a, const TDigest::WeightedPoint& b) { 
            return a < b.value(); 
        }
    );
    size_t Z_index = (Z - centroids_.begin());

    TDigest::WeightedPoint* left = NULL;
    TDigest::WeightedPoint* right = NULL;
    if (Z_index == centroidCount_) {
        left = &centroids_[Z_index - 1];
    }
    else {
        right = &centroids_[Z_index];
        double dr = abs(right->value() - value);
        if (Z_index > 0) {
            left = &centroids_[Z_index - 1];
            double dl = abs(left->value() - value);
            if (dl < dr) {
                right = NULL;                
            } else if (dl > dr) {
                left = NULL;
            }
        }
    }

    // Check size bounds
    //      "This set is reduced by retaining only centroids whose k-size after adding wn would meet the size bound."
    double wl = (Z_index > 0) ? weightLeft(Z_index-1) : 0;

    totalWeight_ += weight;
    if (left) {
        double k0 = scalingK(wl/totalWeight_, delta_);
        wl += left->weight();
        if (scalingK((wl + weight)/totalWeight_, delta_) >= k0 + 1) {
            left = NULL;
        }
    }
    if (right) {
        double k1 = scalingK(wl/totalWeight_, delta_);
        if (scalingK((wl + right->weight() + weight)/totalWeight_, delta_) >= k1 + 1) {
            right = NULL;
        }
    }
    //      "If more than one centroid remains, the one with maximum weight is selected."
    if (left && (!right || (left->weight() >= right->weight()))) {
        left->add(value, weight);
    } 
    else if (right) {
        right->add(value, weight);
    }
    else {
        // insert new centorid
        for (size_t i=centroidCount_; i>Z_index; --i) {
            centroids_[i].set(centroids_[i-1]);
        }
        centroids_[Z_index].set(value, weight);
        ++centroidCount_;

        if (centroidCount_ == centroids_.size()) {
            shrink();
        }
    }
}

void TDigest::shrink() 
{        
    double qlimit = scalingKInverse(1, delta_);
    double weight = centroids_[0].weight();
    double value = centroids_[0].value();
    
    double qleft = 0;
    size_t newCentroidCount = 0;
    for (size_t i=1; i < centroidCount_; ++i) {        
        double wi = centroids_[i].weight();
        double vi = centroids_[i].value();
        double q = qleft + (weight + wi)/totalWeight_;
        if (q <= qlimit) {
            weight += wi;
            value += wi*(vi - value)/weight;
        }
        else {
            centroids_[newCentroidCount].set(value, weight);
            qleft += weight/totalWeight_;
            ++newCentroidCount;
            qlimit = scalingKInverse(newCentroidCount + 1, delta_);
            weight = wi;
            value = vi;
        }
    }
    centroids_[newCentroidCount].set(value, weight);
    centroidCount_ = newCentroidCount + 1;
}

double TDigest::quantile(double q) const 
{
    if (centroidCount_ == 0) {
        return 0.0;
    }

    double rank = q * totalWeight_;

    size_t pos;
    double t;
    if (q > 0.5) {
        if (q >= 1.0) {
            return max_;
        }
        pos = 0;
        t = totalWeight_;
        for (size_t i=centroidCount_; i>0; --i) {
            t -= centroids_[i-1].weight();
            if (rank >= t) {
                pos = i - 1;
                break;
            }
        }
    } else {
        if (q <= 0.0) {
            return min_;
        }
        pos = centroidCount_ - 1;
        t = 0;
        for (size_t i=0; i<centroidCount_; ++i) {
            if (rank < t + centroids_[i].weight()) {
                pos = i;
                break;
            }
            t += centroids_[i].weight();
        }
    }

    double delta = 0;
    double min = min_;
    double max = max_;
    if (centroidCount_ > 1) {
        if (pos == 0) {
            delta = centroids_[pos + 1].value() - centroids_[pos].value();
            max = centroids_[pos + 1].value();
        } else if (pos == centroidCount_ - 1) {
            delta = centroids_[pos].value() - centroids_[pos - 1].value();
            min = centroids_[pos - 1].value();
        } else {
            delta = (centroids_[pos + 1].value() - centroids_[pos - 1].value()) / 2;
            min = centroids_[pos - 1].value();
            max = centroids_[pos + 1].value();
        }
    }
    auto value = centroids_[pos].value() +
        ((rank - t) / centroids_[pos].weight() - 0.5) * delta;

    return (value > max) ? max : ((value < min) ? min : value);
}


}