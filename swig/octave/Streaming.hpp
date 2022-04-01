// Copyright (c) 2022 Nicholas Corgan
// SPDX-License-Identifier: BSL-1.0

#pragma once

#include "Constants.hpp"

#include <SoapySDR/Device.hpp>

#include <octave/CMatrix.h>
#include <octave/fMatrix.h>

#include <cassert>
#include <stdexcept>
#include <vector>

namespace SoapySDR { namespace Octave {

namespace detail
{

template <typename MatrixType>
void readStream(
    SoapySDR::Device *device,
    SoapySDR::Stream *stream,
    const size_t numSamples,
    const size_t numChannels,
    MatrixType &samplesOut,
    StreamFlag &flagsOut,
    long long &timeNsOut,
    const long timeoutUs)
{
    assert(device);
    assert(stream);
    assert(numSamples > 0);
    assert(numChannels > 0);

    samplesOut.resize2(
        static_cast<octave_idx_type>(numChannels),
        static_cast<octave_idx_type>(numSamples));

    auto *samplesBuff = samplesOut.fortran_vec();
    std::vector<void *> buffs;
    for(size_t chan = 0; chan < numChannels; ++chan)
    {
        buffs.emplace_back(&samplesBuff[chan * numSamples]);
    }

    int flagsCpp = 0;
    const auto readRet = device->readStream(
        stream,
        buffs.data(),
        numSamples,
        flagsCpp,
        timeNsOut,
        timeoutUs);

    flagsOut = static_cast<StreamFlag>(flagsCpp);
}

}

inline void readStreamCF32(
    SoapySDR::Device *device,
    SoapySDR::Stream *stream,
    const size_t numSamples,
    const size_t numChannels,
    FloatComplexMatrix &samplesOut,
    StreamFlag &flagsOut,
    long long &timeNsOut,
    const long timeoutUs)
{
    detail::readStream(
        device,
        stream,
        numSamples,
        numChannels,
        samplesOut,
        flagsOut,
        timeNsOut,
        timeoutUs);
}

inline void readStreamCF64(
    SoapySDR::Device *device,
    SoapySDR::Stream *stream,
    const size_t numSamples,
    const size_t numChannels,
    ComplexMatrix &samplesOut,
    StreamFlag &flagsOut,
    long long &timeNsOut,
    const long timeoutUs)
{
    detail::readStream(
        device,
        stream,
        numSamples,
        numChannels,
        samplesOut,
        flagsOut,
        timeNsOut,
        timeoutUs);
}

}}