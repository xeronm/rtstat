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
#include <algorithm>
#include <vector>
#include "p2.hpp"

namespace rtstat {


void P2::initialize() 
{
    std::sort(markers_.begin(), markers_.end(), 
        [](const Marker& a, const Marker& b) { 
            return a.height() < b.height(); 
        }
    );
            
    size_t i = 1;
    double leftIncrement = 0;
    double rightIncrement = 0;
    for (std::vector<Marker>::iterator it=markers_.begin(); it!=markers_.end(); ++it, ++i) {
        double increment = 0;
        if ((i % 2) == 0) {
            // even marker                                  
            size_t qidx = (i + 1)/2 - 1;
            rightIncrement = (qidx < qcount_) ? quantiles_.at(qidx) : 1;
            increment = (leftIncrement + rightIncrement)/2;
        } else {
            // odd marker
            increment = leftIncrement = rightIncrement;
        }

        double desiredPosition = 1 + 2*(qcount_ + 1)*increment;
        it->init((double) i, increment, desiredPosition);
        printf("initialize n:%f, q:%f, inc=%f, dp=%f\n", it->position(), it->height(), increment, desiredPosition);
    }
};

void P2::add(double val)
{
    // Stage A. Initialization
    if (valuesLeftForInit_) {
        --valuesLeftForInit_;
        markers_.at(valuesLeftForInit_).setHeight(val);

        if (!valuesLeftForInit_) {
            initialize();
        }

        return;
    }

    // Stage B. Add observations
    std::vector<Marker>::iterator K = std::lower_bound (markers_.begin(), markers_.end(), val,
        [](const P2::Marker& a, const double b) { 
            return a.height() < b; 
        }
    );
    size_t k_index = (K - markers_.begin()) + 1;
    printf("stage_B: id=%d h=%f (val=%f)\n", k_index, K->height(), val);

    if (K == markers_.begin()) {
        markers_.front().height = val;
    }
    else if (K == markers_.end()) {
        markers_.back().height = val;
    }

    size_t i = 1;
    double q_left, n_left = markers_.at(0).position(), markers_.at(0).position();

    double q_center = 
    Marker left = markers_.at(0);
    Marker center = markers_.at(1);
    for (std::vector<Marker>::iterator it=markers_.begin() + 2; it!=markers_.end(); ++it, ++i) {
        Marker right = *it;

        center.desiredPosition += center.increment;
        if (i >= k_index) ++center.position;

        double d = center.desiredPosition - center.position;
        double dp = left.desiredPosition - 

        left = center;
        center = right;
    }
    center.incPosition(true);
};    

}