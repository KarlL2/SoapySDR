// Copyright (c) 2020-2021 Nicholas Corgan
// SPDX-License-Identifier: BSL-1.0

using System;
using System.Collections.Generic;
using System.Linq;
using System.Runtime.InteropServices;

namespace SoapySDR
{
    internal class Utility
    {
        // TODO: compare stream type to buffer type
        public static void ValidateBuffs<T>(
            StreamHandle streamHandle,
            T[][] buffs) where T: unmanaged
        {
            var numChannels = streamHandle.GetChannels().Count;
            var format = streamHandle.GetFormat();

            if(buffs == null)
            {
                throw new ArgumentNullException("buffs");
            }
            else if(buffs.Length != numChannels)
            {
                throw new ArgumentException(string.Format("Expected {0} channels. Found {1} buffers.", numChannels, buffs.Length));
            }
            else if(!format.Equals(GetFormatString<T>()))
            {
                throw new ArgumentException(string.Format("Expected format \"{0}\". Found format \"{1}\"", GetFormatString<T>(), format));
            }

            HashSet<int> uniqueSizes = new HashSet<int>();

            for(int buffIndex = 0; buffIndex < buffs.Length; ++buffIndex)
            {
                if(buffs[buffIndex] == null)
                {
                    throw new ArgumentNullException(string.Format("buffs[{0}]", buffIndex));
                }

                uniqueSizes.Add(buffs[buffIndex].Length);
            }

            if(uniqueSizes.Count > 1)
            {
                throw new ArgumentException("All buffers must be of the same length.");
            }
        }

        internal static unsafe void ManagedArraysToSizeList<T>(
            T[][] buffs,
            out GCHandle[] handles,
            out SizeList sizeList)
        {
            handles = new GCHandle[buffs.Length];
            sizeList = new SizeList();

            for(int buffIndex = 0; buffIndex < buffs.Length; ++buffIndex)
            {
                handles[buffIndex] = GCHandle.Alloc(
                    buffs[buffIndex],
                    System.Runtime.InteropServices.GCHandleType.Pinned);

                var uptr = (UIntPtr)(void*)handles[buffIndex].AddrOfPinnedObject();
#if _64BIT
                sizeList.Add((ulong)uptr);
#else
                sizeList.Add((uint)uptr);
#endif
            }
        }

        // TODO: complex version
        public static string GetFormatString<T>() where T: unmanaged
        {
            var type = typeof(T);

            if(typeof(T).Equals(typeof(sbyte)))       return StreamFormats.S8;
            else if(typeof(T).Equals(typeof(short)))  return StreamFormats.S16;
            else if(typeof(T).Equals(typeof(int)))    return StreamFormats.S32;
            else if(typeof(T).Equals(typeof(byte)))   return StreamFormats.U8;
            else if(typeof(T).Equals(typeof(ushort))) return StreamFormats.U16;
            else if(typeof(T).Equals(typeof(uint)))   return StreamFormats.U32;
            else if(typeof(T).Equals(typeof(float)))  return StreamFormats.F32;
            else if(typeof(T).Equals(typeof(double))) return StreamFormats.F64;
            else throw new Exception(string.Format("Type {0} not covered by GetFormatString", type));
        }

        public static Kwargs ToKwargs(IDictionary<string, string> input)
        {
            Kwargs kwargs;

            var output = new Kwargs();
            foreach(var pair in input)
            {
                output.Add(pair.Key, pair.Value);
            }

            return output;
        }

        public static Dictionary<string, string> ToDictionary(Kwargs kwargs) => kwargs.ToDictionary(entry => entry.Key, entry => entry.Value);

        public static Dictionary<string, string>[] ToDictionaryArray(KwargsList kwargsList) => kwargsList.Select(x => x.ToDictionary(entry => entry.Key, entry => entry.Value)).ToArray();

        public static ArgInfo[] ToArgInfoArray(ArgInfoInternalList argInfoInternalList) => argInfoInternalList.Select(x => new ArgInfo(x)).ToArray();

        public static Range[] ToRangeArray(RangeInternalList rangeInternalList) => rangeInternalList.Select(x => new Range(x)).ToArray();
    }
}
