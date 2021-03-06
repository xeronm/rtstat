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

class P2
{
    public:
        explicit P2(std::vector<double> quantiles)
            : quantiles_(std::vector<double>(quantiles))
        {
            std:sort(quantiles_.begin(), quantiles_.end());
            qcount_ = quantiles.size();
            markerCount_ = qcount_*2 + 3;
            valuesLeftForInit_ = markerCount_;
            markers_ = std::vector<Marker>(markerCount_);
        };

        void add(double val);
        bool valid() const; // return true if estimation is valid
        double quantile(unsigned char qindex) const;
        double min() const;
        double max() const;
        double count() const; // observations count

        void describe(FILE * f);

    private:
        class Marker
        {
            public:
                explicit Marker()
                    : position(0.0), height(0.0), increment(0.0), desiredPosition(0.0) {};

            inline void init(); // Stage A
            inline void incrementPositions(bool actual); // Stage B.3        
            inline void adjust(Marker& prev, Marker& next); // Stage B.4

            double height; // Estimated quantile value (qi)
            double position; // Marker position (ni)
            double desiredPosition; // Desired marker position (di)
            double increment; // Marker position increment (fi)
        };

        void initialize();

        std::vector<Marker> markers_;
        std::vector<double> quantiles_;
        size_t valuesLeftForInit_; // Observation values left for initialization
        unsigned char qcount_; // Quantiles count for estimate
        unsigned char markerCount_; // Markers count
};

} // namespace rtstat