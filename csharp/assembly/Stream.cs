// Copyright (c) 2020-2021 Nicholas Corgan
// SPDX-License-Identifier: BSL-1.0

using System;
using System.Collections.Generic;

namespace SoapySDR
{
    public class Stream
    {
        internal Device _device = null;
        internal StreamHandle _streamHandle = null;
        protected bool _active = false;

        public string Format { get; }
        public uint[] Channels { get; }
        public Dictionary<string, string> StreamArgs { get; }
        public bool Active { get { return _active; } }

        // We already used these parameters to create the stream,
        // this is just for the sake of getters.
        internal Stream(
            Device device,
            string format,
            uint[] channels,
            KwargsInternal kwargs)
        {
            _device = device;

            Format = format;
            Channels = channels;
            StreamArgs = Utility.ToDictionary(kwargs);
        }

        /*
        ~Stream()
        {
            if(_active)               Deactivate();
            if(_streamHandle != null) Close();
        }
        */

        public ulong MTU => _device.GetStreamMTUInternal(_streamHandle);

        public ErrorCode Activate(
            StreamFlags flags,
            long timeNs = 0,
            uint numElems = 0)
        {
            ErrorCode ret;
            if(_streamHandle != null)
            {
                if(!_active)
                {
                    ret = _device.ActivateStreamInternal(
                        _streamHandle,
                        flags,
                        timeNs,
                        numElems);

                    if (ret == ErrorCode.None) _active = true;
                }
                else throw new InvalidOperationException("Stream is already active");
            }
            else throw new InvalidOperationException("Stream is closed");

            return ret;
        }

        public ErrorCode Deactivate(
            StreamFlags flags = StreamFlags.None,
            long timeNs = 0)
        {
            ErrorCode ret;
            if(_streamHandle != null)
            {
                if(!_active)
                {
                    ret = _device.DeactivateStreamInternal(
                        _streamHandle,
                        flags,
                        timeNs);

                    if(ret == ErrorCode.None) _active = true;
                }
                else throw new InvalidOperationException("Stream is already inactive");
            }
            else throw new InvalidOperationException("Stream is closed");

            return ret;
        }

        public void Close()
        {
            if(_streamHandle != null) _device.CloseStreamInternal(_streamHandle);
            else throw new InvalidOperationException("Stream is already closed");
        }

        //
        // Object overrides
        //

        // For completeness, but a stream is only ever equal to itself
        public override bool Equals(object other) => ReferenceEquals(this, other);

        public override int GetHashCode() => GetType().GetHashCode() ^ (_streamHandle?.GetHashCode() ?? 0);

        public override string ToString()
        {
            return string.Format("{0}:{1} {2} stream (format: {3}, channels: {4})",
                _device.GetDriverKey(),
                _device.GetHardwareKey(),
                (_active ? "active" : "inactive"),
                Format,
                Channels);
        }
    }
}
