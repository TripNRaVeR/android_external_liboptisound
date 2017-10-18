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

#ifndef OPTISOUND_BUTTERWORTH_H
#define OPTISOUND_BUTTERWORTH_H

#include "Common.h"
#include "Cascade.h"
#include "PoleFilter.h"
#include "State.h"

namespace Iir {
namespace Butterworth {
class AnalogLowPass : public LayoutBase {
public:
    AnalogLowPass ();
    void design (const int numPoles);
private:
    int m_numPoles;
};

class AnalogLowShelf : public LayoutBase {
public:
    AnalogLowShelf ();
    void design (int numPoles, double gainDb);
private:
    int m_numPoles;
    double m_gainDb;
};

struct LowPassBase : PoleFilterBase <AnalogLowPass>
{
    void setup (int order,
                double sampleRate,
                double cutoffFrequency);
};

struct HighPassBase : PoleFilterBase <AnalogLowPass>
{
    void setup (int order,
                double sampleRate,
                double cutoffFrequency);
};

struct BandPassBase : PoleFilterBase <AnalogLowPass>
{
    void setup (int order,
                double sampleRate,
                double centerFrequency,
                double widthFrequency);
};

struct BandStopBase : PoleFilterBase <AnalogLowPass>
{
    void setup (int order,
                double sampleRate,
                double centerFrequency,
                double widthFrequency);
};

struct LowShelfBase : PoleFilterBase <AnalogLowShelf>
{
    void setup (int order,
                double sampleRate,
                double cutoffFrequency,
                double gainDb);
};

struct HighShelfBase : PoleFilterBase <AnalogLowShelf>
{
    void setup (int order,
                double sampleRate,
                double cutoffFrequency,
                double gainDb);
};

struct BandShelfBase : PoleFilterBase <AnalogLowShelf>
{
    void setup (int order,
                double sampleRate,
                double centerFrequency,
                double widthFrequency,
                double gainDb);
};

template <int MaxOrder, class StateType = DEFAULT_STATE>
struct LowPass : PoleFilter <LowPassBase, StateType, MaxOrder>
{
};
template <int MaxOrder, class StateType = DEFAULT_STATE>
struct HighPass : PoleFilter <HighPassBase, StateType, MaxOrder>
{
};
template <int MaxOrder, class StateType = DEFAULT_STATE>
struct BandPass : PoleFilter <BandPassBase, StateType, MaxOrder, MaxOrder*2>
{
};
template <int MaxOrder, class StateType = DEFAULT_STATE>
struct BandStop : PoleFilter <BandStopBase, StateType, MaxOrder, MaxOrder*2>
{
};
template <int MaxOrder, class StateType = DEFAULT_STATE>
struct LowShelf : PoleFilter <LowShelfBase, StateType, MaxOrder>
{
};
template <int MaxOrder, class StateType = DEFAULT_STATE>
struct HighShelf : PoleFilter <HighShelfBase, StateType, MaxOrder>
{
};
template <int MaxOrder, class StateType = DEFAULT_STATE>
struct BandShelf : PoleFilter <BandShelfBase, StateType, MaxOrder, MaxOrder*2>
{
};
}
}
#endif
