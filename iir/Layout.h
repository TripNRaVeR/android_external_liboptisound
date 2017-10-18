/*
 * Copyright (C) 2017 TripNDroid Mobile Engineering
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef OPTISOUND_LAYOUT_H
#define OPTISOUND_LAYOUT_H

#include "Common.h"
#include "MathSupplement.h"

namespace Iir
{
class LayoutBase
{
public:
    LayoutBase ()
        : m_numPoles (0)
        , m_maxPoles (0)
    {
    }
    LayoutBase (int maxPoles, PoleZeroPair* pairs)
        : m_numPoles (0)
        , m_maxPoles (maxPoles)
        , m_pair (pairs)
    {
    }
    void setStorage (const LayoutBase& other)
    {
        m_numPoles = 0;
        m_maxPoles = other.m_maxPoles;
        m_pair = other.m_pair;
    }
    void reset ()
    {
        m_numPoles = 0;
    }
    int getNumPoles () const
    {
        return m_numPoles;
    }
    int getMaxPoles () const
    {
        return m_maxPoles;
    }
    void add (const complex_t& pole, const complex_t& zero)
    {
        m_pair[m_numPoles/2] = PoleZeroPair (pole, zero);
        ++m_numPoles;
    }
    void addPoleZeroConjugatePairs (const complex_t pole,
                                    const complex_t zero)
    {
        m_pair[m_numPoles/2] = PoleZeroPair (
                                   pole, zero, std::conj (pole), std::conj (zero));
        m_numPoles += 2;
    }
    void add (const ComplexPair& poles, const ComplexPair& zeros)
    {
        m_pair[m_numPoles/2] = PoleZeroPair (poles.first, zeros.first,
                                             poles.second, zeros.second);
        m_numPoles += 2;
    }
    const PoleZeroPair& getPair (int pairIndex) const
    {
        return m_pair[pairIndex];
    }
    const PoleZeroPair& operator[] (int pairIndex) const
    {
        return getPair (pairIndex);
    }
    double getNormalW () const
    {
        return m_normalW;
    }
    double getNormalGain () const
    {
        return m_normalGain;
    }
    void setNormal (double w, double g)
    {
        m_normalW = w;
        m_normalGain = g;
    }
private:
    int m_numPoles;
    int m_maxPoles;
    PoleZeroPair* m_pair;
    double m_normalW;
    double m_normalGain;
};
template <int MaxPoles>
class Layout
{
public:
    operator LayoutBase ()
    {
        return LayoutBase (MaxPoles, m_pairs);
    }
private:
    PoleZeroPair m_pairs[(MaxPoles+1)/2];
};
}
#endif
