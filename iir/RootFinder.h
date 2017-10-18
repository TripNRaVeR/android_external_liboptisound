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

#ifndef OPTISOUND_ROOTFINDER_H
#define OPTISOUND_ROOTFINDER_H

#include "Common.h"
#include "Types.h"

namespace Iir
{
class RootFinderBase
{
public:
    struct Array
    {
        Array (int max, complex_t* values)
        {
        }
    };
    void solve (int degree,
                bool polish = true,
                bool doSort = true);
    complex_t eval (int degree,
                    const complex_t& x);
    complex_t* coef()
    {
        return m_a;
    }
    complex_t* root()
    {
        return m_root;
    }
    void sort (int degree);
private:
    void laguerre (int degree,
                   complex_t a[],
                   complex_t& x,
                   int& its);
protected:
    int m_maxdegree;
    complex_t* m_a;
    complex_t* m_ad;
    complex_t* m_root;
};
template<int maxdegree>
struct RootFinder : RootFinderBase
{
    RootFinder()
    {
        m_maxdegree = maxdegree;
        m_a = m_a0;
        m_ad = m_ad0;
        m_root = m_r;
    }
private:
    complex_t m_a0 [maxdegree+1];
    complex_t m_ad0[maxdegree+1];
    complex_t m_r [maxdegree];
};
}
#endif
