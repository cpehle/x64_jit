// Copyright 2014, the V8 project authors. All rights reserved.
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are
// met:
//
//     * Redistributions of source code must retain the above copyright
//       notice, this list of conditions and the following disclaimer.
//     * Redistributions in binary form must reproduce the above
//       copyright notice, this list of conditions and the following
//       disclaimer in the documentation and/or other materials provided
//       with the distribution.
//     * Neither the name of Google Inc. nor the names of its
//       contributors may be used to endorse or promote products derived
//       from this software without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
// "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
// LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
// A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
// OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
// SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
// LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
// DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
// THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
// (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
// OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

// The original source code covered by the above license above has been
// modified significantly by Google Inc.
// Copyright 2017 Google Inc. All rights reserved.

#include <utility>
#include <cstdint>
#include <cstring>

#include "jit/cpu.h"


namespace sling {
namespace jit {

bool CPU::initialized = false;
unsigned CPU::features = 0;
unsigned CPU::cache_line_size = 0;
bool CPU::vzero_needed = false;

static void __cpuid(int cpu_info[4], int info_type) {
  __asm__ volatile("cpuid \n\t"
                   : "=a"(cpu_info[0]), "=b"(cpu_info[1]), "=c"(cpu_info[2]),
                     "=d"(cpu_info[3])
                   : "a"(info_type), "c"(0));
}

static uint64_t _xgetbv(unsigned int xcr) {
  unsigned eax, edx;
  __asm__ volatile(".byte 0x0f, 0x01, 0xd0" : "=a"(eax), "=d"(edx) : "c"(xcr));
  return static_cast<uint64_t>(eax) | (static_cast<uint64_t>(edx) << 32);
}

static bool os_has_avx_support() {
  // Get XFEATURE_ENABLED_MASK register.
  uint64_t feature_mask = _xgetbv(0);

  // Check for AVX support.
  return (feature_mask & 0x6) == 0x6;
}

ProcessorInformation::ProcessorInformation() {
  memcpy(vendor_, "Unknown", 8);
  memcpy(brand_, "Unknown", 8);
  int cpu_info[4];

  // Query basic CPU information.
  __cpuid(cpu_info, 0);
  unsigned num_ids = cpu_info[0];
  std::swap(cpu_info[2], cpu_info[3]);
  memcpy(vendor_, cpu_info + 1, 12);
  vendor_[12] = '\0';

  // Interpret CPU feature information.
  if (num_ids > 0) {
    __cpuid(cpu_info, 1);
    stepping_ = cpu_info[0] & 0xf;
    model_ = ((cpu_info[0] >> 4) & 0xf) + ((cpu_info[0] >> 12) & 0xf0);
    family_ = (cpu_info[0] >> 8) & 0xf;
    type_ = (cpu_info[0] >> 12) & 0x3;
    ext_model_ = (cpu_info[0] >> 16) & 0xf;
    ext_family_ = (cpu_info[0] >> 20) & 0xff;
    has_fpu_ = (cpu_info[3] & 0x00000001) != 0;
    has_cmov_ = (cpu_info[3] & 0x00008000) != 0;
    has_mmx_ = (cpu_info[3] & 0x00800000) != 0;
    has_sse_ = (cpu_info[3] & 0x02000000) != 0;
    has_sse2_ = (cpu_info[3] & 0x04000000) != 0;
    has_sse3_ = (cpu_info[2] & 0x00000001) != 0;
    has_ssse3_ = (cpu_info[2] & 0x00000200) != 0;
    has_sse41_ = (cpu_info[2] & 0x00080000) != 0;
    has_sse42_ = (cpu_info[2] & 0x00100000) != 0;
    has_f16c_ = (cpu_info[2] & 0x20000000) != 0;
    has_popcnt_ = (cpu_info[2] & 0x00800000) != 0;
    has_osxsave_ = (cpu_info[2] & 0x08000000) != 0;
    has_avx_ = (cpu_info[2] & 0x10000000) != 0;
    has_fma3_ = (cpu_info[2] & 0x00001000) != 0;
  }

  // There are separate feature flags for VEX-encoded GPR instructions.
  if (num_ids >= 7) {
    __cpuid(cpu_info, 7);
    has_bmi1_ = (cpu_info[1] & 0x00000008) != 0;
    has_bmi2_ = (cpu_info[1] & 0x00000100) != 0;
    has_avx2_ = (cpu_info[1] & 0x00000020) != 0;
  }

  // Query extended IDs.
  __cpuid(cpu_info, 0x80000000);
  unsigned num_ext_ids = cpu_info[0];

  // Get CPU brand.
  if (num_ext_ids > 0x80000000) {
    int brand[12];
    __cpuid(brand + 0, 0x80000002);
    __cpuid(brand + 4, 0x80000003);
    __cpuid(brand + 8, 0x80000004);
    char *p = reinterpret_cast<char *>(brand);
    char *end = p + 3 * 4 * 4;
    char *q = brand_;
    bool space = false;
    while (p < end && *p == ' ') p++;
    while (p < end) {
      if (*p == ' ') {
        space = true;
      } else {
        if (space) *q++ = ' ';
        space = false;
        *q++ = *p;
      }
      p++;
    }
    *q = 0;
  }

  // Interpret extended CPU feature information.
  if (num_ext_ids > 0x80000000) {
    __cpuid(cpu_info, 0x80000001);
    has_lzcnt_ = (cpu_info[2] & 0x00000020) != 0;
    // SAHF must be probed in long mode.
    has_sahf_ = (cpu_info[2] & 0x00000001) != 0;
  }

  // Sandy Bridge and later have fast zero idiom (PXORx reg,reg).
  if (family_model() >= 0x062A) has_zero_idiom_ = true;

  // Skylake and later have fast one idiom (PCMPEQx reg,reg).
  if (family_model() >= 0x065E) has_one_idiom_ = true;

  // Get cache line size.
  if (strcmp(vendor_, "GenuineIntel") == 0) {
    __cpuid(cpu_info, 0x00000001);
    cache_line_size_ =  ((cpu_info[1] >> 8) & 0xff) * 8;
  } else if (strcmp(vendor_, "AuthenticAMD") == 0) {
    __cpuid(cpu_info, 0x80000005);
    cache_line_size_ =  cpu_info[2] & 0xff;
  } else {
    cache_line_size_ = 64;
  }
}

const char *ProcessorInformation::architecture() {
  switch (family_model()) {
    case 0x065E:
      return "Skylake";

    case 0x063D:
      return "Broadwell";

    case 0x063C:
    case 0x063F:
    case 0x0645:
    case 0x0646:
      return "Haswell";

    case 0x064A:
      return "Silvermont";

    case 0x063A:
    case 0x063E:
      return "Ivy Bridge";

    case 0x062A:
    case 0x062D:
      return "Sandy Bridge";

    case 0x061E:
    case 0x061A:
    case 0x062E:
      return "Nehalem";

    case 0x0625:
    case 0x062C:
    case 0x062F:
      return "Westmere";

    case 0x0617:
    case 0x061D:
      return "Penryn";

    case 0x060F:
    case 0x0616:
      return "Merom";

    case 0x0F06:
      return "Presler";

    case 0x0F03:
    case 0x0F04:
      return "Prescott";

    case 0x060D:
      return "Dothan";

    default:
      return "";
  }
}

void CPU::Initialize() {
  ProcessorInformation cpu;

  if (cpu.has_mmx()) features |= 1u << MMX;
  if (cpu.has_sse()) features |= 1u << SSE;
  if (cpu.has_sse2()) features |= 1u << SSE2;
  if (cpu.has_sse3()) features |= 1u << SSE3;
  if (cpu.has_ssse3()) features |= 1u << SSSE3;
  if (cpu.has_sse41()) features |= 1u << SSE4_1;
  if (cpu.has_f16c()) features |= 1u << F16C;
  if (cpu.has_sahf()) features |= 1u << SAHF;

  if (cpu.has_osxsave() && os_has_avx_support()) {
    features |= 1u << AVX;
    if (cpu.has_fma3()) features |= 1u << FMA3;
    if (cpu.has_avx2()) features |= 1u << AVX2;
  }

  if (cpu.has_bmi1()) features |= 1u << BMI1;
  if (cpu.has_bmi2()) features |= 1u << BMI2;
  if (cpu.has_lzcnt()) features |= 1u << LZCNT;
  if (cpu.has_popcnt()) features |= 1u << POPCNT;

  if (cpu.has_zero_idiom()) features |= 1u << ZEROIDIOM;
  if (cpu.has_one_idiom()) features |= 1u << ONEIDIOM;

  cache_line_size = cpu.cache_line_size();

  vzero_needed = false;
  if (cpu.has_avx()) {
#ifndef __AVX__
    vzero_needed = true;
#endif
  }
}

}  // namespace jit
}  // namespace sling

