// Copyright (c) 2021 Nicholas Corgan
// SPDX-License-Identifier: BSL-1.0

#pragma once

#include "Constants.hpp"

#include <SoapySDR/Device.hpp>

#include <algorithm>
#include <cassert>
#include <cstdio>
#include <memory>
#include <utility>

using StreamResultPairInternal = std::pair<SoapySDR::CSharp::ErrorCode, SoapySDR::CSharp::StreamResult>;

// We're separately using a separate thin wrapper for three reasons:
// * To abstract away the make() and unmake() calls, which SWIG won't
//   deal with well.
// * To avoid exposing a function without converting its naming convention
//   to that of C#.
// * Any other processing we need for a better C# API.
namespace SoapySDR { namespace CSharp {

    struct DeviceDeleter
    {
        void operator()(SoapySDR::Device* pDevice)
        {
            if(pDevice)
            {
                try { SoapySDR::Device::unmake(pDevice); }
                catch(const std::exception& ex) { fputs(ex.what(), stderr); }
                catch(...) { fputs("Unknown error.", stderr); }
            }
        }
    };

    class DeviceInternal
    {
        public:
            DeviceInternal(const Kwargs& kwargs): _deviceSPtr(SoapySDR::Device::make(kwargs), DeviceDeleter())
            {}

            DeviceInternal(const std::string& args): _deviceSPtr(SoapySDR::Device::make(args), DeviceDeleter())
            {}

            static inline SoapySDR::KwargsList Enumerate()
            {
                return SoapySDR::Device::enumerate();
            }

            static inline SoapySDR::KwargsList Enumerate(const std::string& args)
            {
                return SoapySDR::Device::enumerate(args);
            }

            static inline SoapySDR::KwargsList Enumerate(const SoapySDR::Kwargs& args)
            {
                return SoapySDR::Device::enumerate(args);
            }

            //
            // Identification API
            //

            inline std::string GetDriverKey() const
            {
                assert(_deviceSPtr);

                return _deviceSPtr->getDriverKey();
            }

            inline std::string GetHardwareKey() const
            {
                assert(_deviceSPtr);

                return _deviceSPtr->getHardwareKey();
            }

            inline SoapySDR::Kwargs GetHardwareInfo() const
            {
                assert(_deviceSPtr);

                return _deviceSPtr->getHardwareInfo();
            }

            //
            // Channels API
            //

            inline void SetFrontendMapping(
                const SoapySDR::CSharp::Direction direction,
                const std::string& mapping)
            {
                assert(_deviceSPtr);

                return _deviceSPtr->setFrontendMapping(int(direction), mapping);
            }

            inline std::string GetFrontendMapping(SoapySDR::CSharp::Direction direction) const
            {
                assert(_deviceSPtr);

                return _deviceSPtr->getFrontendMapping(int(direction));
            }

            inline size_t GetNumChannels(SoapySDR::CSharp::Direction direction) const
            {
                assert(_deviceSPtr);

                return _deviceSPtr->getNumChannels(int(direction));
            }

            inline SoapySDR::Kwargs GetChannelInfo(
                const SoapySDR::CSharp::Direction direction,
                const size_t channel) const
            {
                assert(_deviceSPtr);

                return _deviceSPtr->getChannelInfo(int(direction), channel);
            }

            inline bool GetFullDuplex(
                const SoapySDR::CSharp::Direction direction,
                const size_t channel) const
            {
                assert(_deviceSPtr);

                return _deviceSPtr->getFullDuplex(int(direction), channel);
            }

            //
            // Stream API
            //

            inline std::vector<std::string> GetStreamFormats(
                const SoapySDR::CSharp::Direction direction,
                const size_t channel) const
            {
                assert(_deviceSPtr);

                return _deviceSPtr->getStreamFormats(int(direction), channel);
            }

            inline std::string GetNativeStreamFormat(
                const SoapySDR::CSharp::Direction direction,
                const size_t channel,
                double &fullScaleOut) const
            {
                assert(_deviceSPtr);

                return _deviceSPtr->getNativeStreamFormat(int(direction), channel, fullScaleOut);
            }

            inline SoapySDR::ArgInfoList GetStreamArgsInfo(
                const SoapySDR::CSharp::Direction direction,
                const size_t channel) const
            {
                assert(_deviceSPtr);

                return _deviceSPtr->getStreamArgsInfo(int(direction), channel);
            }

            SoapySDR::CSharp::StreamHandle SetupStream(
                const SoapySDR::CSharp::Direction direction,
                const std::string& format,
                const SWIGSizeVector& channels,
                const SoapySDR::Kwargs& kwargs)
            {
                assert(_deviceSPtr);

                SoapySDR::CSharp::StreamHandle streamHandle;
                streamHandle.stream = _deviceSPtr->setupStream(int(direction), format, copyVector<size_t>(channels), kwargs);
                streamHandle.format = format;
                streamHandle.channels = channels;

                return streamHandle;
            }

            inline void CloseStream(const SoapySDR::CSharp::StreamHandle& streamHandle)
            {
                assert(_deviceSPtr);

                _deviceSPtr->closeStream(streamHandle.stream);
            }

            inline size_t GetStreamMTU(const SoapySDR::CSharp::StreamHandle& streamHandle)
            {
                assert(_deviceSPtr);

                return _deviceSPtr->getStreamMTU(streamHandle.stream);
            }

            inline SoapySDR::CSharp::ErrorCode ActivateStream(
                const SoapySDR::CSharp::StreamHandle& streamHandle,
                const SoapySDR::CSharp::StreamFlags flags,
                const long long timeNs,
                const size_t numElems)
            {
                assert(_deviceSPtr);

                return SoapySDR::CSharp::ErrorCode(_deviceSPtr->activateStream(
                    streamHandle.stream,
                    int(flags),
                    timeNs,
                    numElems));
            }

            inline SoapySDR::CSharp::ErrorCode DeactivateStream(
                const SoapySDR::CSharp::StreamHandle& streamHandle,
                const SoapySDR::CSharp::StreamFlags flags,
                const long long timeNs)
            {
                assert(_deviceSPtr);

                return SoapySDR::CSharp::ErrorCode(_deviceSPtr->deactivateStream(
                    streamHandle.stream,
                    int(flags),
                    timeNs));
            }

            StreamResultPairInternal ReadStream(
                const SoapySDR::CSharp::StreamHandle& streamHandle,
                const SWIGSizeVector& buffs,
                const size_t numElems,
                const SoapySDR::CSharp::StreamFlags flags,
                const long long timeNs,
                const long timeoutUs)
            {
                assert(_deviceSPtr);

                StreamResultPairInternal resultPair;
                auto& errorCode = resultPair.first;
                auto& result = resultPair.second;

                const auto buffPtrs = reinterpretCastVector<void>(buffs);
                auto intFlags = int(flags);
                auto cppRet = _deviceSPtr->readStream(
                    streamHandle.stream,
                    buffPtrs.data(),
                    numElems,
                    intFlags,
                    result.TimeNs,
                    result.TimeoutUs);
                result.Flags = SoapySDR::CSharp::StreamFlags(intFlags);

                if(cppRet >= 0) result.NumSamples = static_cast<size_t>(cppRet);
                else            errorCode = static_cast<SoapySDR::CSharp::ErrorCode>(cppRet);

                return resultPair;
            }

            StreamResultPairInternal WriteStream(
                const SoapySDR::CSharp::StreamHandle& streamHandle,
                const SWIGSizeVector& buffs,
                const size_t numElems,
                const long long timeNs,
                const long timeoutUs)
            {
                assert(_deviceSPtr);

                StreamResultPairInternal resultPair;
                auto& errorCode = resultPair.first;
                auto& result = resultPair.second;

                const auto buffPtrs = reinterpretCastVector<const void>(buffs);
                int intFlags = 0;
                auto cppRet = _deviceSPtr->writeStream(
                    streamHandle.stream,
                    buffPtrs.data(),
                    numElems,
                    intFlags,
                    timeNs,
                    timeoutUs);
                result.Flags = SoapySDR::CSharp::StreamFlags(intFlags);

                if(cppRet >= 0) result.NumSamples = static_cast<size_t>(cppRet);
                else            errorCode = static_cast<SoapySDR::CSharp::ErrorCode>(cppRet);

                return resultPair;
            }

            StreamResultPairInternal ReadStreamStatus(
                const SoapySDR::CSharp::StreamHandle& streamHandle,
                const long timeoutUs)
            {
                assert(_deviceSPtr);

                StreamResultPairInternal resultPair;
                auto& errorCode = resultPair.first;
                auto& result = resultPair.second;

                int intFlags = 0;
                errorCode = SoapySDR::CSharp::ErrorCode(_deviceSPtr->readStreamStatus(
                    streamHandle.stream,
                    result.ChanMask,
                    intFlags,
                    result.TimeNs,
                    result.TimeoutUs));
                result.Flags = SoapySDR::CSharp::StreamFlags(intFlags);

                return resultPair;
            }

            //
            // Antenna API
            //

            inline std::vector<std::string> ListAntennas(
                const SoapySDR::CSharp::Direction direction,
                const size_t channel) const
            {
                assert(_deviceSPtr);

                return _deviceSPtr->listAntennas(int(direction), channel);
            }

            inline void SetAntenna(
                const SoapySDR::CSharp::Direction direction,
                const size_t channel,
                const std::string& name)
            {
                assert(_deviceSPtr);

                _deviceSPtr->setAntenna(int(direction), channel, name);
            }

            inline std::string GetAntenna(
                const SoapySDR::CSharp::Direction direction,
                const size_t channel) const
            {
                assert(_deviceSPtr);

                return _deviceSPtr->getAntenna(int(direction), channel);
            }

            //
            // Frontend corrections API
            //

            inline bool HasDCOffsetMode(
                const SoapySDR::CSharp::Direction direction,
                const size_t channel) const
            {
                assert(_deviceSPtr);

                return _deviceSPtr->hasDCOffsetMode(int(direction), channel);
            }

            inline void SetDCOffsetMode(
                const SoapySDR::CSharp::Direction direction,
                const size_t channel,
                const bool automatic)
            {
                assert(_deviceSPtr);

                return _deviceSPtr->setDCOffsetMode(int(direction), channel, automatic);
            }

            inline bool GetDCOffsetMode(
                const SoapySDR::CSharp::Direction direction,
                const size_t channel)
            {
                assert(_deviceSPtr);

                return _deviceSPtr->getDCOffsetMode(int(direction), channel);
            }

            inline bool HasDCOffset(
                const SoapySDR::CSharp::Direction direction,
                const size_t channel) const
            {
                assert(_deviceSPtr);

                return _deviceSPtr->hasDCOffset(int(direction), channel);
            }

            inline void SetDCOffset(
                const SoapySDR::CSharp::Direction direction,
                const size_t channel,
                const std::complex<double>& offset)
            {
                assert(_deviceSPtr);

                _deviceSPtr->setDCOffset(int(direction), channel, offset);
            }

            inline std::complex<double> GetDCOffset(
                const SoapySDR::CSharp::Direction direction,
                const size_t channel) const
            {
                assert(_deviceSPtr);

                return _deviceSPtr->getDCOffset(int(direction), channel);
            }

            inline bool HasIQBalance(
                const SoapySDR::CSharp::Direction direction,
                const size_t channel) const
            {
                assert(_deviceSPtr);

                return _deviceSPtr->hasIQBalance(int(direction), channel);
            }

            inline void SetIQBalance(
                const SoapySDR::CSharp::Direction direction,
                const size_t channel,
                const std::complex<double>& balance)
            {
                assert(_deviceSPtr);

                _deviceSPtr->setIQBalance(int(direction), channel, balance);
            }

            inline std::complex<double> GetIQBalance(
                const SoapySDR::CSharp::Direction direction,
                const size_t channel) const
            {
                assert(_deviceSPtr);

                return _deviceSPtr->getIQBalance(int(direction), channel);
            }

            inline bool HasIQBalanceMode(
                const SoapySDR::CSharp::Direction direction,
                const size_t channel) const
            {
                assert(_deviceSPtr);

                return _deviceSPtr->hasIQBalanceMode(int(direction), channel);
            }

            inline void SetIQBalanceMode(
                const SoapySDR::CSharp::Direction direction,
                const size_t channel,
                const bool automatic)
            {
                assert(_deviceSPtr);

                _deviceSPtr->setIQBalanceMode(int(direction), channel, automatic);
            }

            inline bool GetIQBalanceMode(
                const SoapySDR::CSharp::Direction direction,
                const size_t channel) const
            {
                assert(_deviceSPtr);

                return _deviceSPtr->getIQBalanceMode(int(direction), channel);
            }

            inline bool HasFrequencyCorrection(
                const SoapySDR::CSharp::Direction direction,
                const size_t channel) const
            {
                assert(_deviceSPtr);

                return _deviceSPtr->hasFrequencyCorrection(int(direction), channel);
            }

            inline void SetFrequencyCorrection(
                const SoapySDR::CSharp::Direction direction,
                const size_t channel,
                const double value)
            {
                assert(_deviceSPtr);

                _deviceSPtr->setFrequencyCorrection(int(direction), channel, value);
            }

            inline double GetFrequencyCorrection(
                const SoapySDR::CSharp::Direction direction,
                const size_t channel) const
            {
                assert(_deviceSPtr);

                return _deviceSPtr->getFrequencyCorrection(int(direction), channel);
            }

            //
            // Gain API
            //

            inline std::vector<std::string> ListGains(
                const SoapySDR::CSharp::Direction direction,
                const size_t channel)
            {
                assert(_deviceSPtr);

                return _deviceSPtr->listGains(int(direction), channel);
            }

            inline bool HasGainMode(
                const SoapySDR::CSharp::Direction direction,
                const size_t channel) const
            {
                assert(_deviceSPtr);

                return _deviceSPtr->hasGainMode(int(direction), channel);
            }

            inline void SetGainMode(
                const SoapySDR::CSharp::Direction direction,
                const size_t channel,
                const bool automatic)
            {
                assert(_deviceSPtr);

                _deviceSPtr->setGainMode(int(direction), channel, automatic);
            }

            inline bool GetGainMode(
                const SoapySDR::CSharp::Direction direction,
                const size_t channel) const
            {
                assert(_deviceSPtr);

                return _deviceSPtr->getGainMode(int(direction), channel);
            }

            inline void SetGain(
                const SoapySDR::CSharp::Direction direction,
                const size_t channel,
                const double value)
            {
                assert(_deviceSPtr);

                _deviceSPtr->setGain(int(direction), channel, value);
            }

            inline void SetGain(
                const SoapySDR::CSharp::Direction direction,
                const size_t channel,
                const std::string& name,
                const double value)
            {
                assert(_deviceSPtr);

                _deviceSPtr->setGain(int(direction), channel, name, value);
            }

            inline double GetGain(
                const SoapySDR::CSharp::Direction direction,
                const size_t channel)
            {
                assert(_deviceSPtr);

                return _deviceSPtr->getGain(int(direction), channel);
            }

            inline double GetGain(
                const SoapySDR::CSharp::Direction direction,
                const size_t channel,
                const std::string& name)
            {
                assert(_deviceSPtr);

                return _deviceSPtr->getGain(int(direction), channel, name);
            }

            inline SoapySDR::Range GetGainRange(
                const SoapySDR::CSharp::Direction direction,
                const size_t channel)
            {
                assert(_deviceSPtr);

                return _deviceSPtr->getGainRange(int(direction), channel);
            }

            inline SoapySDR::Range GetGainRange(
                const SoapySDR::CSharp::Direction direction,
                const size_t channel,
                const std::string& name)
            {
                assert(_deviceSPtr);

                return _deviceSPtr->getGainRange(int(direction), channel, name);
            }

            //
            // Frequency API
            //

            inline void SetFrequency(
                const SoapySDR::CSharp::Direction direction,
                const size_t channel,
                const double frequency,
                const SoapySDR::Kwargs& args)
            {
                assert(_deviceSPtr);

                _deviceSPtr->setFrequency(int(direction), channel, frequency, args);
            }

            inline void SetFrequency(
                const SoapySDR::CSharp::Direction direction,
                const size_t channel,
                const std::string& name,
                const double frequency,
                const SoapySDR::Kwargs& args)
            {
                assert(_deviceSPtr);

                _deviceSPtr->setFrequency(int(direction), channel, name, frequency, args);
            }

            inline double GetFrequency(
                const SoapySDR::CSharp::Direction direction,
                const size_t channel) const
            {
                assert(_deviceSPtr);

                return _deviceSPtr->getFrequency(int(direction), channel);
            }

            inline double GetFrequency(
                const SoapySDR::CSharp::Direction direction,
                const size_t channel,
                const std::string& name) const
            {
                assert(_deviceSPtr);

                return _deviceSPtr->getFrequency(int(direction), channel, name);
            }

            inline std::vector<std::string> ListFrequencies(
                const SoapySDR::CSharp::Direction direction,
                const size_t channel) const
            {
                assert(_deviceSPtr);

                return _deviceSPtr->listFrequencies(int(direction), channel);
            }

            inline SoapySDR::RangeList GetFrequencyRange(
                const SoapySDR::CSharp::Direction direction,
                const size_t channel) const 
            {
                assert(_deviceSPtr);

                return _deviceSPtr->getFrequencyRange(int(direction), channel);
            }

            inline SoapySDR::RangeList GetFrequencyRange(
                const SoapySDR::CSharp::Direction direction,
                const size_t channel,
                const std::string& name) const
            {
                assert(_deviceSPtr);

                return _deviceSPtr->getFrequencyRange(int(direction), channel, name);
            }

            inline SoapySDR::ArgInfoList GetFrequencyArgsInfo(
                const SoapySDR::CSharp::Direction direction,
                const size_t channel) const
            {
                assert(_deviceSPtr);

                return _deviceSPtr->getFrequencyArgsInfo(int(direction), channel);
            }

            //
            // Sample Rate API
            //

            inline void SetSampleRate(
                const SoapySDR::CSharp::Direction direction,
                const size_t channel,
                const double rate)
            {
                assert(_deviceSPtr);

                _deviceSPtr->setSampleRate(int(direction), channel, rate);
            }

            inline double GetSampleRate(
                const SoapySDR::CSharp::Direction direction,
                const size_t channel) const
            {
                assert(_deviceSPtr);

                return _deviceSPtr->getSampleRate(int(direction), channel);
            }

            inline SoapySDR::RangeList GetSampleRateRange(
                const SoapySDR::CSharp::Direction direction,
                const size_t channel) const
            {
                assert(_deviceSPtr);

                return _deviceSPtr->getSampleRateRange(int(direction), channel);
            }

            //
            // Bandwidth API
            //

            inline void SetBandwidth(
                const SoapySDR::CSharp::Direction direction,
                const size_t channel,
                const double bandwidth)
            {
                assert(_deviceSPtr);

                _deviceSPtr->setBandwidth(int(direction), channel, bandwidth);
            }

            inline double GetBandwidth(
                const SoapySDR::CSharp::Direction direction,
                const size_t channel) const
            {
                assert(_deviceSPtr);

                return _deviceSPtr->getBandwidth(int(direction), channel);
            }

            inline SoapySDR::RangeList GetBandwidthRange(
                const SoapySDR::CSharp::Direction direction,
                const size_t channel) const
            {
                assert(_deviceSPtr);

                return _deviceSPtr->getBandwidthRange(int(direction), channel);
            }

            //
            // Clocking API
            //

            inline void SetMasterClockRate(const double rate)
            {
                assert(_deviceSPtr);

                _deviceSPtr->setMasterClockRate(rate);
            }

            inline double GetMasterClockRate() const
            {
                assert(_deviceSPtr);

                return _deviceSPtr->getMasterClockRate();
            }

            inline SoapySDR::RangeList GetMasterClockRates() const
            {
                assert(_deviceSPtr);

                return _deviceSPtr->getMasterClockRates();
            }

            inline void SetReferenceClockRate(const double rate)
            {
                assert(_deviceSPtr);

                _deviceSPtr->setReferenceClockRate(rate);
            }

            inline double GetReferenceClockRate() const
            {
                assert(_deviceSPtr);

                return _deviceSPtr->getReferenceClockRate();
            }

            inline SoapySDR::RangeList GetReferenceClockRates() const
            {
                assert(_deviceSPtr);

                return _deviceSPtr->getReferenceClockRates();
            }

            inline std::vector<std::string> ListClockSources() const
            {
                assert(_deviceSPtr);

                return _deviceSPtr->listClockSources();
            }

            inline void SetClockSource(const std::string& source)
            {
                assert(_deviceSPtr);

                _deviceSPtr->setClockSource(source);
            }

            inline std::string GetClockSource() const
            {
                assert(_deviceSPtr);

                return _deviceSPtr->getClockSource();
            }

            //
            // Time API
            //

            inline std::vector<std::string> ListTimeSources() const
            {
                assert(_deviceSPtr);

                return _deviceSPtr->listTimeSources();
            }

            inline void SetTimeSource(const std::string& source)
            {
                assert(_deviceSPtr);

                _deviceSPtr->setTimeSource(source);
            }

            inline std::string GetTimeSource() const
            {
                assert(_deviceSPtr);

                return _deviceSPtr->getTimeSource();
            }

            inline bool HasHardwareTime(const std::string& what) const
            {
                assert(_deviceSPtr);

                return _deviceSPtr->hasHardwareTime(what);
            }

            inline long long GetHardwareTime(const std::string& what) const
            {
                assert(_deviceSPtr);

                return _deviceSPtr->getHardwareTime(what);
            }

            inline void SetHardwareTime(const long long timeNs, const std::string& what)
            {
                assert(_deviceSPtr);

                _deviceSPtr->setHardwareTime(timeNs, what);
            }

            //
            // Sensor API
            //

            inline std::vector<std::string> ListSensors() const
            {
                assert(_deviceSPtr);

                return _deviceSPtr->listSensors();
            }

            inline SoapySDR::ArgInfo GetSensorInfo(const std::string& key) const
            {
                assert(_deviceSPtr);

                return _deviceSPtr->getSensorInfo(key);
            }

            inline std::string ReadSensor(const std::string& key) const
            {
                assert(_deviceSPtr);

                return _deviceSPtr->readSensor(key);
            }

            inline std::vector<std::string> ListSensors(
                const SoapySDR::CSharp::Direction direction,
                const size_t channel) const
            {
                assert(_deviceSPtr);

                return _deviceSPtr->listSensors(int(direction), channel);
            }

            inline SoapySDR::ArgInfo GetSensorInfo(
                const SoapySDR::CSharp::Direction direction,
                const size_t channel,
                const std::string& key) const
            {
                assert(_deviceSPtr);

                return _deviceSPtr->getSensorInfo(int(direction), channel, key);
            }

            inline std::string ReadSensor(
                const SoapySDR::CSharp::Direction direction,
                const size_t channel,
                const std::string& key) const
            {
                assert(_deviceSPtr);

                return _deviceSPtr->readSensor(int(direction), channel, key);
            }

            //
            // Register API
            //

            inline std::vector<std::string> ListRegisterInterfaces() const
            {
                assert(_deviceSPtr);

                return _deviceSPtr->listRegisterInterfaces();
            }

            inline void WriteRegister(
                const std::string& name,
                const unsigned addr,
                const unsigned value)
            {
                assert(_deviceSPtr);

                _deviceSPtr->writeRegister(name, addr, value);
            }

            inline unsigned ReadRegister(
                const std::string& name,
                const unsigned addr) const
            {
                assert(_deviceSPtr);

                return _deviceSPtr->readRegister(name, addr);
            }

            // To avoid the uint/ulong issue, these functions will internally
            // use SWIGSizeVector. The public-facing function will use uint[] as
            // expected, and these are less commonly used functions, so we can
            // begrudgingly accept this performance hit.

            inline void WriteRegisters(
                const std::string& name,
                const unsigned addr,
                const SWIGSizeVector& value)
            {
                assert(_deviceSPtr);

                _deviceSPtr->writeRegisters(
                    name,
                    addr,
                    copyVector<unsigned>(value));
            }

            inline SWIGSizeVector ReadRegisters(
                const std::string& name,
                const unsigned addr,
                const size_t length)
            {
                assert(_deviceSPtr);

                return copyVector<SWIGSize>(_deviceSPtr->readRegisters(name, addr, length));
            }

            //
            // Settings API
            //

            inline SoapySDR::ArgInfoList GetSettingInfo() const
            {
                assert(_deviceSPtr);

                return _deviceSPtr->getSettingInfo();
            }

            inline void WriteSetting(
                const std::string& key,
                const std::string& value)
            {
                assert(_deviceSPtr);

                _deviceSPtr->writeSetting(key, value);
            }

            inline std::string ReadSetting(const std::string& key) const
            {
                assert(_deviceSPtr);

                return _deviceSPtr->readSetting(key);
            }

            inline SoapySDR::ArgInfoList GetSettingInfo(
                const SoapySDR::CSharp::Direction direction,
                const size_t channel) const
            {
                assert(_deviceSPtr);

                return _deviceSPtr->getSettingInfo(int(direction), channel);
            }

            inline void WriteSetting(
                const SoapySDR::CSharp::Direction direction,
                const size_t channel,
                const std::string& key,
                const std::string& value)
            {
                assert(_deviceSPtr);

                _deviceSPtr->writeSetting(int(direction), channel, key, value);
            }

            inline std::string ReadSetting(
                const SoapySDR::CSharp::Direction direction,
                const size_t channel,
                const std::string& key) const
            {
                assert(_deviceSPtr);

                return _deviceSPtr->readSetting(int(direction), channel, key);
            }

            //
            // GPIO API
            //

            inline std::vector<std::string> ListGPIOBanks() const
            {
                assert(_deviceSPtr);

                return _deviceSPtr->listGPIOBanks();
            }

            inline void WriteGPIO(
                const std::string& bank,
                const unsigned value)
            {
                assert(_deviceSPtr);

                _deviceSPtr->writeGPIO(bank, value);
            }

            inline void WriteGPIO(
                const std::string& bank,
                const unsigned value,
                const unsigned mask)
            {
                assert(_deviceSPtr);

                _deviceSPtr->writeGPIO(bank, value, mask);
            }

            inline unsigned ReadGPIO(const std::string& bank) const
            {
                assert(_deviceSPtr);

                return _deviceSPtr->readGPIO(bank);
            }

            inline void WriteGPIODir(
                const std::string& bank,
                const unsigned dir)
            {
                assert(_deviceSPtr);

                _deviceSPtr->writeGPIODir(bank, dir);
            }

            inline void WriteGPIODir(
                const std::string& bank,
                const unsigned dir,
                const unsigned mask)
            {
                assert(_deviceSPtr);

                _deviceSPtr->writeGPIODir(bank, dir, mask);
            }

            inline unsigned ReadGPIODir(const std::string& bank) const
            {
                assert(_deviceSPtr);

                return _deviceSPtr->readGPIODir(bank);
            }

            //
            // I2C API
            //

            inline void WriteI2C(
                const int addr,
                const std::string& data)
            {
                assert(_deviceSPtr);

                _deviceSPtr->writeI2C(addr, data);
            }

            inline std::string ReadI2C(
                const int addr,
                const size_t numBytes)
            {
                assert(_deviceSPtr);

                return _deviceSPtr->readI2C(addr, numBytes);
            }

            //
            // SPI API
            //

            inline unsigned TransactSPI(
                const int addr,
                const unsigned data,
                const size_t numBits)
            {
                assert(_deviceSPtr);

                return _deviceSPtr->transactSPI(addr, data, numBits);
            }

            //
            // UART API
            //

            inline std::vector<std::string> ListUARTs() const
            {
                assert(_deviceSPtr);

                return _deviceSPtr->listUARTs();
            }

            inline void WriteUART(
                const std::string& which,
                const std::string& data)
            {
                assert(_deviceSPtr);

                _deviceSPtr->writeUART(which, data);
            }

            inline std::string ReadUART(
                const std::string& which,
                const long long timeoutUs) const
            {
                assert(_deviceSPtr);

                return _deviceSPtr->readUART(which, static_cast<long>(timeoutUs));
            }

            //
            // Used for CSharp internals
            //

            inline std::string __ToString() const
            {
                assert(_deviceSPtr);

                return (_deviceSPtr->getDriverKey() + ":" + _deviceSPtr->getHardwareKey());
            }

            inline bool Equals(const SoapySDR::CSharp::DeviceInternal& other) const
            {
                assert(_deviceSPtr);

                return (__ToString() == other.__ToString());
            }

            inline SWIGSize GetPointer() const
            {
                assert(_deviceSPtr);

                return reinterpret_cast<SWIGSize>(_deviceSPtr.get());
            }

        private:
            std::shared_ptr<SoapySDR::Device> _deviceSPtr;

            // C# class takes ownership, will unmake
            DeviceInternal(SoapySDR::Device* pDevice): _deviceSPtr(pDevice, DeviceDeleter())
            {}
    };
}}
