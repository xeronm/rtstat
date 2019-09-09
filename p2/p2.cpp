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
#include "p2.hpp"

namespace rtstat {

inline void P2::Marker::incrementPositions(bool actual) {
    desiredPosition += increment;
    if (actual) {
        ++position;
    }
}

inline void P2::Marker::adjust(P2::Marker& prev, P2::Marker& next) {
    double d = desiredPosition - position;
    double dp = next.position - position;
    double dm = prev.position - position;

    if ((d >= 1) && (dp > 1)) {
        double qp = (next.height - height)/dp;
        double qm = (prev.height - height)/dm;
        double qt = height + ((1 - dm)*qp + (dp - 1)*qm)/(dp - dm);
        if ((qt > prev.height) && (qt < next.height)) {
            height = qt;
        }
        else {
            height += qp;
        }
        ++position;
    }
    else if ((d <= -1) && (dm < -1)) {
        double qp = (next.height - height)/dp;
        double qm = (prev.height - height)/dm;
        double qt = height - ((1 + dp)*qm - (dm + 1)*qp)/(dp - dm);
        if ((qt > prev.height) && (qt < next.height)) {
            height = qt;
        }
        else {
            height -= qm;
        }
        --position;
    }
}

void P2::describe(FILE * f) 
{
    fprintf(f, "quantiles: %d - ", qcount_);
    for (auto it=quantiles_.begin(); it!=quantiles_.end(); ++it) {
        fprintf(f, " %0.5f", *it);
    }
    fprintf(f, "\nmarkers: %d, min:%10.4f, max:%10.4f\n        pos     height    qantile\n", markerCount_, markers_.front().position, markers_.back().position);
    unsigned char i = 0;
    for (auto it=markers_.begin(); it!=markers_.end(); ++it, ++i) {
        if ((i % 2 == 0) && (i > 0) && (i < markerCount_ - 1) ) {
            fprintf(f, " %10.4f %10.4f %10.4f\n", it->position, it->height, quantiles_[i/2 - 1]);
        } 
        else {
            if (i == 0) {
                fprintf(f, " %10.4f %10.4f     min\n", it->position, it->height);
            }
            else if (i == markerCount_ - 1) {
                fprintf(f, " %10.4f %10.4f     max\n", it->position, it->height);
            }
            else {
                fprintf(f, " %10.4f %10.4f\n", it->position, it->height);
            }
        }
    }
}

void P2::initialize() 
{
    std::sort(markers_.begin(), markers_.end(), 
        [](const P2::Marker& a, const P2::Marker& b) { 
            return a.height < b.height; 
        }
    );
            
    size_t i = 1;
    double leftIncrement = 0;
    double rightIncrement = 0;
    for (auto it=markers_.begin(); it!=markers_.end(); ++it, ++i) {
        double increment = 0;
        if ((i % 2) == 0) {
            // even marker                                  
            size_t qidx = (i + 1)/2 - 1;
            rightIncrement = (qidx < qcount_) ? quantiles_[qidx] : 1;
            increment = (leftIncrement + rightIncrement)/2;
        } else {
            // odd marker
            increment = leftIncrement = rightIncrement;
        }

        it->position = i;
        it->desiredPosition = 1 + 2*(qcount_ + 1)*increment;
        it->increment = increment;
    }
};

void P2::add(double val)
{
    // Stage A. Initialization
    if (valuesLeftForInit_) {
        --valuesLeftForInit_;
        markers_[valuesLeftForInit_].height = val;

        if (!valuesLeftForInit_) {
            initialize();
        }

        return;
    }

    // Stage B. Add observations
    auto K = std::upper_bound (markers_.begin(), markers_.end(), val,
        [](const double a, const P2::Marker& b) { 
            return a < b.height; 
        }
    );
    size_t k_index = (K - markers_.begin());
    if (k_index == 0) {
        // set MIN marker
        markers_[0].height = val;
        ++k_index;
    }
    else if (k_index == markerCount_) {
        // set MAX marker
        markers_[markerCount_-1].height = val;
    }

    P2::Marker* prev = &markers_[0];
    P2::Marker* curr = &markers_[1];
    for (size_t i=2; i<markerCount_; ++i) {
        P2::Marker* next = &markers_[i];

        // According to B.1-2 
        //    curr marker index 
        //       ci = (i - 1)
        //    actual position must incremeted only if 
        //       ci >= k + 1
        //    because of upper_bounds
        //       k_index = k + 1
        curr->incrementPositions(i > k_index); 
        curr->adjust(*prev, *next);

        prev = curr;
        curr = next;
    }
    curr->incrementPositions(true);

};    

bool P2::valid() const 
{
    return (valuesLeftForInit_ == 0);
}

double P2::quantile(unsigned char qindex) const 
{
    if ((qindex < 0) || (qindex > qcount_)) {
        return 0;
    }
    return markers_[qindex*2 + 2].height;
}

double P2::min() const
{
    return markers_[0].height;
};

double P2::max() const
{
    return markers_[markerCount_].height;
};

double P2::count() const
{
    return markers_[markerCount_].position;
};


}