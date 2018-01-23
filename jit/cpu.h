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

#ifndef JIT_CPU_H_
#define JIT_CPU_H_


namespace sling {
namespace jit {

// Processor information.
class ProcessorInformation {
 public:
  // Query CPU about supported features.
  ProcessorInformation();

  // x86 CPU ID information.
  const char *vendor() const { return vendor_; }
  const char *brand() const { return brand_; }
  int stepping() const { return stepping_; }
  int model() const { return model_; }
  int ext_model() const { return ext_model_; }
  int family() const { return family_; }
  int ext_family() const { return ext_family_; }
  int type() const { return type_; }
  const char *architecture();
  int family_model() const { return (family_ << 8) | model_; }

  // General features.
  bool has_fpu() const { return has_fpu_; }
  int cache_line_size() const { return cache_line_size_; }
  static const int UNKNOWN_CACHE_LINE_SIZE = 0;

  // x86 features.
  bool has_cmov() const { return has_cmov_; }
  bool has_sahf() const { return has_sahf_; }
  bool has_mmx() const { return has_mmx_; }
  bool has_sse() const { return has_sse_; }
  bool has_sse2() const { return has_sse2_; }
  bool has_sse3() const { return has_sse3_; }
  bool has_ssse3() const { return has_ssse3_; }
  bool has_sse41() const { return has_sse41_; }
  bool has_sse42() const { return has_sse42_; }
  bool has_f16c() const { return has_f16c_; }
  bool has_osxsave() const { return has_osxsave_; }
  bool has_avx() const { return has_avx_; }
  bool has_avx2() const { return has_avx2_; }
  bool has_fma3() const { return has_fma3_; }
  bool has_bmi1() const { return has_bmi1_; }
  bool has_bmi2() const { return has_bmi2_; }
  bool has_lzcnt() const { return has_lzcnt_; }
  bool has_popcnt() const { return has_popcnt_; }
  bool has_zero_idiom() const { return has_zero_idiom_; }
  bool has_one_idiom() const { return has_one_idiom_; }

 private:
  char vendor_[13];
  char brand_[49];
  int stepping_ = 0;
  int model_ = 0;
  int ext_model_ = 0;
  int family_ = 0;
  int ext_family_ = 0;
  int type_ = 0;
  int cache_line_size_ = UNKNOWN_CACHE_LINE_SIZE;
  bool has_fpu_ = false;
  bool has_cmov_ = false;
  bool has_sahf_ = false;
  bool has_mmx_ = false;
  bool has_sse_ = false;
  bool has_sse2_ = false;
  bool has_sse3_ = false;
  bool has_ssse3_ = false;
  bool has_sse41_ = false;
  bool has_sse42_ = false;
  bool has_f16c_ = false;
  bool has_osxsave_ = false;
  bool has_avx_ = false;
  bool has_avx2_ = false;
  bool has_fma3_ = false;
  bool has_bmi1_ = false;
  bool has_bmi2_ = false;
  bool has_lzcnt_ = false;
  bool has_popcnt_ = false;
  bool has_zero_idiom_ = false;
  bool has_one_idiom_ = false;
};

// CPU feature flags.
enum CpuFeature {
  MMX,
  SSE,
  SSE2,
  SSE3,
  SSSE3,
  SSE4_1,
  SSE4_2,
  F16C,
  AVX,
  AVX2,
  FMA3,
  SAHF,
  BMI1,
  BMI2,
  LZCNT,
  POPCNT,
  ZEROIDIOM,
  ONEIDIOM,

  NUMBER_OF_CPU_FEATURES,
};

// Keep track of which features are supported by the target CPU. Individual
// features can be enabled and disabled on a global basis.
class CPU {
 public:
  // Probe CPU for supported features.
  static void Probe() {
    if (initialized) return;
    Initialize();
    initialized = true;
  }

  // Return bit mask with supported features.
  static unsigned SupportedFeatures() {
    Probe();
    return features;
  }

  // Check if CPU feature is enabled.
  static bool Enabled(CpuFeature f) {
    Probe();
    return (features & (1u << f)) != 0;
  }

  // Enable CPU feature.
  static void Enable(CpuFeature f) {
    Probe();
    features |= (1u << f);
  }

  // Disable CPU feature.
  static void Disable(CpuFeature f) {
    Probe();
    features &= ~(1u << f);
  }

  // Cache line size.
  static unsigned CacheLineSize() {
    Probe();
    return cache_line_size;
  }

  // VZEROUPPER is only needed on some processors.
  static bool VZeroNeeded() {
    Probe();
    return vzero_needed;
  }

 private:
  // Initialize CPU features by querying the CPU.
  static void Initialize();

  // CPU features that are enabled.
  static unsigned features;

  // Cache line size.
  static unsigned cache_line_size;

  // VZEROUPPER needed on AVX/SSE transitions.
  static bool vzero_needed;

  // CPU features are only probed once.
  static bool initialized;
};

}  // namespace jit
}  // namespace sling

#endif  // JIT_CPU_H_

