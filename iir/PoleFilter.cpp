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

#include "Common.h"
#include "PoleFilter.h"

namespace Iir
{
complex_t LowPassTransform::transform (complex_t c)
{
    if (c == infinity())
        return complex_t (-1, 0);
    c = f * c;
    return (1. + c) / (1. - c);
}
LowPassTransform::LowPassTransform (double fc,
                                    LayoutBase& digital,
                                    LayoutBase const& analog)
{
    digital.reset ();
    f = tan (doublePi * fc);
    const int numPoles = analog.getNumPoles ();
    const int pairs = numPoles / 2;
    for (int i = 0; i < pairs; ++i)
    {
        const PoleZeroPair& pair = analog[i];
        digital.addPoleZeroConjugatePairs (transform (pair.poles.first),
                                           transform (pair.zeros.first));
    }
    if (numPoles & 1)
    {
        const PoleZeroPair& pair = analog[pairs];
        digital.add (transform (pair.poles.first),
                     transform (pair.zeros.first));
    }
    digital.setNormal (analog.getNormalW(),
                       analog.getNormalGain());
}
complex_t HighPassTransform::transform (complex_t c)
{
    if (c == infinity())
        return complex_t (1, 0);
    c = f * c;
    return - (1. + c) / (1. - c);
}
HighPassTransform::HighPassTransform (double fc,
                                      LayoutBase& digital,
                                      LayoutBase const& analog)
{
    digital.reset ();
    f = 1. / tan (doublePi * fc);
    const int numPoles = analog.getNumPoles ();
    const int pairs = numPoles / 2;
    for (int i = 0; i < pairs; ++i)
    {
        const PoleZeroPair& pair = analog[i];
        digital.addPoleZeroConjugatePairs (transform (pair.poles.first),
                                           transform (pair.zeros.first));
    }
    if (numPoles & 1)
    {
        const PoleZeroPair& pair = analog[pairs];
        digital.add (transform (pair.poles.first),
                     transform (pair.zeros.first));
    }
    digital.setNormal (doublePi - analog.getNormalW(),
                       analog.getNormalGain());
}
BandPassTransform::BandPassTransform (double fc,
                                      double fw,
                                      LayoutBase& digital,
                                      LayoutBase const& analog)
{
    digital.reset ();
    const double ww = 2 * doublePi * fw;
    wc2 = 2 * doublePi * fc - (ww / 2);
    wc = wc2 + ww;
    if (wc2 < 1e-8)
        wc2 = 1e-8;
    if (wc > doublePi-1e-8)
        wc = doublePi-1e-8;
    a = cos ((wc + wc2) * 0.5) /
            cos ((wc - wc2) * 0.5);
    b = 1 / tan ((wc - wc2) * 0.5);
    a2 = a * a;
    b2 = b * b;
    ab = a * b;
    ab_2 = 2 * ab;
    const int numPoles = analog.getNumPoles ();
    const int pairs = numPoles / 2;
    for (int i = 0; i < pairs; ++i)
    {
        const PoleZeroPair& pair = analog[i];
        ComplexPair p1 = transform (pair.poles.first);
        ComplexPair z1 = transform (pair.zeros.first);
        digital.addPoleZeroConjugatePairs (p1.first, z1.first);
        digital.addPoleZeroConjugatePairs (p1.second, z1.second);
    }
    if (numPoles & 1)
    {
        ComplexPair poles = transform (analog[pairs].poles.first);
        ComplexPair zeros = transform (analog[pairs].zeros.first);
        digital.add (poles, zeros);
    }
    double wn = analog.getNormalW();
    digital.setNormal (2 * atan (sqrt (tan ((wc + wn)* 0.5) * tan((wc2 + wn)* 0.5))),
                       analog.getNormalGain());
}
ComplexPair BandPassTransform::transform (complex_t c)
{
    if (c == infinity())
        return ComplexPair (-1, 1);
    c = (1. + c) / (1. - c);
    complex_t v = 0;
    v = addmul (v, 4 * (b2 * (a2 - 1) + 1), c);
    v += 8 * (b2 * (a2 - 1) - 1);
    v *= c;
    v += 4 * (b2 * (a2 - 1) + 1);
    v = std::sqrt (v);
    complex_t u = -v;
    u = addmul (u, ab_2, c);
    u += ab_2;
    v = addmul (v, ab_2, c);
    v += ab_2;
    complex_t d = 0;
    d = addmul (d, 2 * (b - 1), c) + 2 * (1 + b);
    return ComplexPair (u/d, v/d);
}
BandStopTransform::BandStopTransform (double fc,
                                      double fw,
                                      LayoutBase& digital,
                                      LayoutBase const& analog)
{
    digital.reset ();
    const double ww = 2 * doublePi * fw;
    wc2 = 2 * doublePi * fc - (ww / 2);
    wc = wc2 + ww;
    if (wc2 < 1e-8)
        wc2 = 1e-8;
    if (wc > doublePi-1e-8)
        wc = doublePi-1e-8;
    a = cos ((wc + wc2) * .5) /
        cos ((wc - wc2) * .5);
    b = tan ((wc - wc2) * .5);
    a2 = a * a;
    b2 = b * b;
    const int numPoles = analog.getNumPoles ();
    const int pairs = numPoles / 2;
    for (int i = 0; i < pairs; ++i)
    {
        const PoleZeroPair& pair = analog[i];
        ComplexPair p = transform (pair.poles.first);
        ComplexPair z = transform (pair.zeros.first);
        if (z.second == z.first)
            z.second = std::conj (z.first);
        digital.addPoleZeroConjugatePairs (p.first, z.first);
        digital.addPoleZeroConjugatePairs (p.second, z.second);
    }
    if (numPoles & 1)
    {
        ComplexPair poles = transform (analog[pairs].poles.first);
        ComplexPair zeros = transform (analog[pairs].zeros.first);
        digital.add (poles, zeros);
    }
    if (fc < 0.25)
        digital.setNormal (doublePi, analog.getNormalGain());
    else
        digital.setNormal (0, analog.getNormalGain());
}
ComplexPair BandStopTransform::transform (complex_t c)
{
    if (c == infinity())
        c = -1;
    else
        c = (1. + c) / (1. - c);
    complex_t u (0);
    u = addmul (u, 4 * (b2 + a2 - 1), c);
    u += 8 * (b2 - a2 + 1);
    u *= c;
    u += 4 * (a2 + b2 - 1);
    u = std::sqrt (u);
    complex_t v = u * -.5;
    v += a;
    v = addmul (v, -a, c);
    u *= .5;
    u += a;
    u = addmul (u, -a, c);
    complex_t d (b + 1);
    d = addmul (d, b-1, c);
    return ComplexPair (u/d, v/d);
}
}
