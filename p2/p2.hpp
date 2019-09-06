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

#pragma once

#include <vector>

namespace rtstat
{

class P2
{
public:
    class Marker
    {
    public:
        explicit Marker()
            : position_(0.0), height_(0.0), increment_(0.0), desiredPosition_(0.0) {};

        inline double height() const
        {
            return height_;
        }

        inline double position() const
        {
            return position_;
        }

        inline void setHeight(double height)
        {
            height_ = height;
        }

        inline double desiredPosition() const
        {
            return desiredPosition_;
        }

        inline void setPosition(double position)
        {
            position_ = position;
        }

        inline void incPosition(bool is_actual)
        {
            if (is_actual) ++position_;
            desiredPosition_ += increment_;
        }

        inline void init(double position, double increment, double desiredPosition)
        {
            position_ = position;
            increment_ = increment;
            desiredPosition_ = desiredPosition;
        }

    private:
        double height_; // Qi
        double position_; // Ni
        double desiredPosition_; // Di
        double increment_; // Fi
    };

    explicit P2(std::vector<double> quantiles)
        : quantiles_(std::vector<double>(quantiles))
    {
        qcount_ = quantiles.size();
        valuesLeftForInit_ = qcount_*2 + 3;
        markers_ = std::vector<Marker>(valuesLeftForInit_);
        printf("created  q:%d, m:%d\n", quantiles_.size(), markers_.size());
    };

    void add(double val);
private:
    std::vector<Marker> markers_;
    std::vector<double> quantiles_;
    size_t valuesLeftForInit_;
    size_t qcount_;

    void initialize();
};

} // namespace rtstat