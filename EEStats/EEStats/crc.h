#pragma once
// Source https://www.atwillys.de/content/cc/cpp-hash-algorithms-class-templates-crc-sha1-sha256-md5/

/**
 * @package de.atwillys.cc.swl
 * @license BSD (simplified)
 * @author Stefan Wilhelm (stfwi)
 *
 * @file crc.hh
 * @ccflags
 * @ldflags
 * @platform linux, bsd, windows
 * @standard >= c++98
 *
 * -----------------------------------------------------------------------------
 *
 * CRC16/32 calculation class template. Not much to say, usage:
 *
 *  uint16_t checksum = sw::crc16::calculate(pointer_to_data, size_of_data);
 *
 *  uint32_t checksum = sw::crc32::calculate(pointer_to_data, size_of_data);
 *
 * -----------------------------------------------------------------------------
 * +++ BSD license header +++
 * Copyright (c) 2008-2014, Stefan Wilhelm (stfwi, <cerbero s@atwilly s.de>)
 * All rights reserved.
 * Redistribution and use in source and binary forms, with or without modification,
 * are permitted provided that the following conditions are met: (1) Redistributions
 * of source code must retain the above copyright notice, this list of conditions
 * and the following disclaimer. (2) Redistributions in binary form must reproduce
 * the above copyright notice, this list of conditions and the following disclaimer
 * in the documentation and/or other materials provided with the distribution.
 * (3) Neither the name of atwillys.de nor the names of its contributors may be
 * used to endorse or promote products derived from this software without specific
 * prior written permission. THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS
 * AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING,
 * BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 * A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER
 * OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT
 * OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
 * STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY
 * WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH
 * DAMAGE.
 * -----------------------------------------------------------------------------
 */

/*
    
     EX MAIN (just in case the page go down)

using namespace std;

int main(int argc, const char* argv[])
{
  // Any kind of data example data types
  struct {
    char bytes[100];
    int something;
    unsigned long whatever;
  } data;

  string path = "/home/me/a-file";

  /////////////////////////////////////////////////////////////
  // SHA1, SHA512 and MD5 static functions return the checksum
  // as hex string.

  // String
  cout << sw::sha1::calculate("SHA of std::string") << endl;

  // Binary data (void*, size_t)
  cout << sw::sha1::calculate(&data, sizeof(data)) << endl;

  // File checksum
  cout << sw::sha1::file(path) << endl;

  // Streams (std::istream &) aer possible, too
  std::stringstream ss("SHA of std::stringstream");
  cout << sw::sha1::calculate(ss) << endl;

  // The same with md5 and sha512
  cout << sw::md5::calculate("MD5 of std::string") << endl;
  cout << sw::sha512::calculate("SHA512 of std::string") << endl;
  cout << sw::sha512::file(path) << endl;
  cout << sw::sha512::calculate(&data, sizeof(data)) << endl;
  // etc, etc.

  /////////////////////////////////////////////////////////////
  // CRC returns a number, not a hex string
  // crc16: uint16_t, crc32: uint32_t
  // `stream` function and `file` function are not exported.
  cout << sw::crc16::calculate("CRC16 of std::string") << endl;
  cout << sw::crc32::calculate("CRC32 of std::string") << endl;
  cout << sw::crc16::calculate(&data, sizeof(data)) << endl;
  cout << sw::crc32::calculate(&data, sizeof(data)) << endl;
  return 0;
}


*/

#include <string>
#include <cstring>
#if defined(OS_WIN) || defined (_WINDOWS_) || defined(_WIN32) || defined(__MSC_VER)
#include <stdint.h>
#else
#include <inttypes.h>
#endif

namespace sw {
    namespace detail {

        /**
         * Type selective lookup tables
         *//**
         * Template class basic_crc
         */
        template <typename acc_type, typename size_type, acc_type initial_crc_value, acc_type final_xor_value>
        class basic_crc
        {
        public:

            /**
             * Calculate CRC16, std::string
             * @param const std::string & s
             * @return acc_type
             */
            static inline acc_type calculate(const std::string& s)
            {
                return calculate(s.c_str(), s.length());
            }

            /**
             * Calculate CRC16, C string
             * @param const char* c_str
             * @return acc_type
             */
            static inline acc_type calculate(const char* c_str)
            {
                return (!c_str) ? 0 : (calculate(c_str, strlen(c_str)));
            }

            /**
             * Calculate CRC16, raw data and length
             * @param const void *data
             * @param size_type size
             * @return acc_type
             */
            static acc_type calculate(const void* data, register size_type size)
            {
                // Compiler is gently asked to as much as possible in registers, but depending
                // on the processor it will not to that.
                acc_type crc = initial_crc_value;
                const unsigned char* p = (const unsigned char*)data;
                if (data) {
                    while (size--) crc = (crc_lookups<acc_type>::tab[((crc) ^ (*p++)) & 0xff] ^ ((crc) >> 8));
                }
                return crc ^ final_xor_value;
            }
        };
    }
}

namespace sw {
    typedef detail::basic_crc<uint16_t, size_t, 0xffff, 0xffff> crc16;
    typedef detail::basic_crc<uint32_t, size_t, 0xffffffff, 0xffffffff> crc32;
}