// Copyright (c) 1994-2006 Sun Microsystems Inc.
// All Rights Reserved.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are
// met:
//
// - Redistributions of source code must retain the above copyright notice,
// this list of conditions and the following disclaimer.
//
// - Redistribution in binary form must reproduce the above copyright
// notice, this list of conditions and the following disclaimer in the
// documentation and/or other materials provided with the distribution.
//
// - Neither the name of Sun Microsystems or the names of contributors may
// be used to endorse or promote products derived from this software without
// specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS
// IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
// PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR
// CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
// EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
// PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
// PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
// LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
// NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
// SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

// The original source code covered by the above license above has been
// modified significantly by Google Inc.
// Copyright 2012 the V8 project authors. All rights reserved.
// Copyright 2017 Google Inc. All rights reserved.

#ifndef JIT_REGISTERS_H_
#define JIT_REGISTERS_H_


namespace sling {
namespace jit {

// x86-64 general registers.
#define GENERAL_REGISTERS(V) \
  V(rax)                     \
  V(rcx)                     \
  V(rdx)                     \
  V(rbx)                     \
  V(rsp)                     \
  V(rbp)                     \
  V(rsi)                     \
  V(rdi)                     \
  V(r8)                      \
  V(r9)                      \
  V(r10)                     \
  V(r11)                     \
  V(r12)                     \
  V(r13)                     \
  V(r14)                     \
  V(r15)

// CPU Registers.
struct Register {
  enum Code {
#define REGISTER_CODE(R) kCode_##R,
    GENERAL_REGISTERS(REGISTER_CODE)
#undef REGISTER_CODE
    kAfterLast,
    kCode_no_reg = -1
  };

  static const int kNumRegisters = Code::kAfterLast;

  static Register from_code(int code) {
    // DCHECK(code >= 0);
    // DCHECK(code < kNumRegisters);
    Register r = {code};
    return r;
  }

  bool is_valid() const { return 0 <= reg_code && reg_code < kNumRegisters; }

  bool is(Register reg) const { return reg_code == reg.reg_code; }

  int code() const {
    // DCHECK(is_valid());
    return reg_code;
  }

  int bit() const {
    // DCHECK(is_valid());
    return 1 << reg_code;
  }

  bool is_byte_register() const { return reg_code <= 3; }

  // Return the high bit of the register code as a 0 or 1. Used often
  // when constructing the REX prefix byte.
  int high_bit() const { return reg_code >> 3; }

  // Return the 3 low bits of the register code. Used when encoding registers
  // in modR/M, SIB, and opcode bytes.
  int low_bits() const { return reg_code & 0x7; }

  // Register code.
  int reg_code;
};


#define DECLARE_REGISTER(R) const Register R = {Register::kCode_##R};
GENERAL_REGISTERS(DECLARE_REGISTER)
#undef DECLARE_REGISTER
const Register no_reg = {Register::kCode_no_reg};

// Registers for first six arguments.
const Register arg_reg_1 = {Register::kCode_rdi};
const Register arg_reg_2 = {Register::kCode_rsi};
const Register arg_reg_3 = {Register::kCode_rdx};
const Register arg_reg_4 = {Register::kCode_rcx};
const Register arg_reg_5 = {Register::kCode_r8};
const Register arg_reg_6 = {Register::kCode_r9};

// SIMD registers.
#define SIMD128_REGISTERS(V) \
  V(xmm0)                   \
  V(xmm1)                   \
  V(xmm2)                   \
  V(xmm3)                   \
  V(xmm4)                   \
  V(xmm5)                   \
  V(xmm6)                   \
  V(xmm7)                   \
  V(xmm8)                   \
  V(xmm9)                   \
  V(xmm10)                  \
  V(xmm11)                  \
  V(xmm12)                  \
  V(xmm13)                  \
  V(xmm14)                  \
  V(xmm15)

struct XMMRegister {
  enum Code {
#define REGISTER_CODE(R) kCode_##R,
    SIMD128_REGISTERS(REGISTER_CODE)
#undef REGISTER_CODE
    kAfterLast,
    kCode_no_reg = -1
  };

  static const int kMaxNumRegisters = Code::kAfterLast;

  static XMMRegister from_code(int code) {
    XMMRegister result = {code};
    return result;
  }

  bool is_valid() const { return 0 <= reg_code && reg_code < kMaxNumRegisters; }

  bool is(XMMRegister reg) const { return reg_code == reg.reg_code; }

  int code() const {
    // DCHECK(is_valid());
    return reg_code;
  }

  // Return the high bit of the register code as a 0 or 1. Used often
  // when constructing the REX prefix byte.
  int high_bit() const { return reg_code >> 3; }

  // Return the 3 low bits of the register code. Used when encoding registers
  // in modR/M, SIB, and opcode bytes.
  int low_bits() const { return reg_code & 0x7; }

  // Register code.
  int reg_code;
};

#define DECLARE_REGISTER(R) const XMMRegister R = {XMMRegister::kCode_##R};
SIMD128_REGISTERS(DECLARE_REGISTER)
#undef DECLARE_REGISTER
const XMMRegister no_xmm_reg = {XMMRegister::kCode_no_reg};

#define SIMD256_REGISTERS(V) \
  V(ymm0)                   \
  V(ymm1)                   \
  V(ymm2)                   \
  V(ymm3)                   \
  V(ymm4)                   \
  V(ymm5)                   \
  V(ymm6)                   \
  V(ymm7)                   \
  V(ymm8)                   \
  V(ymm9)                   \
  V(ymm10)                  \
  V(ymm11)                  \
  V(ymm12)                  \
  V(ymm13)                  \
  V(ymm14)                  \
  V(ymm15)

struct YMMRegister {
  enum Code {
#define REGISTER_CODE(R) kCode_##R,
    SIMD256_REGISTERS(REGISTER_CODE)
#undef REGISTER_CODE
    kAfterLast,
    kCode_no_reg = -1
  };

  static const int kMaxNumRegisters = Code::kAfterLast;

  static YMMRegister from_code(int code) {
    YMMRegister result = {code};
    return result;
  }

  bool is_valid() const { return 0 <= reg_code && reg_code < kMaxNumRegisters; }

  bool is(YMMRegister reg) const { return reg_code == reg.reg_code; }

  XMMRegister xmm() const {
    XMMRegister result = {reg_code};
    return result;
  }

  int code() const {
    // DCHECK(is_valid());
    return reg_code;
  }

  // Return the high bit of the register code as a 0 or 1. Used often
  // when constructing the REX prefix byte.
  int high_bit() const { return reg_code >> 3; }

  // Return the 3 low bits of the register code. Used when encoding registers
  // in modR/M, SIB, and opcode bytes.
  int low_bits() const { return reg_code & 0x7; }

  // Register code.
  int reg_code;
};

#define DECLARE_REGISTER(R) const YMMRegister R = {YMMRegister::kCode_##R};
SIMD256_REGISTERS(DECLARE_REGISTER)
#undef DECLARE_REGISTER
const YMMRegister no_ymm_reg = {YMMRegister::kCode_no_reg};

// Condition flags.
enum Condition {
  // Any value < 0 is considered no_condition
  no_condition  = -1,

  overflow      =  0,
  no_overflow   =  1,
  below         =  2,
  above_equal   =  3,
  equal         =  4,
  not_equal     =  5,
  below_equal   =  6,
  above         =  7,
  negative      =  8,
  positive      =  9,
  parity_even   = 10,
  parity_odd    = 11,
  less          = 12,
  greater_equal = 13,
  less_equal    = 14,
  greater       = 15,

  // Fake conditions that are handled by the opcodes using them.
  always        = 16,
  never         = 17,

  // Aliases.
  carry         = below,
  not_carry     = above_equal,
  zero          = equal,
  not_zero      = not_equal,
  sign          = negative,
  not_sign      = positive,
  last_condition = greater
};

// Returns the equivalent of !cc.
// Negation of the default no_condition (-1) results in a non-default
// no_condition value (-2). As long as tests for no_condition check
// for condition < 0, this will work as expected.
inline Condition NegateCondition(Condition cc) {
  return static_cast<Condition>(cc ^ 1);
}

// Commute a condition such that {a cond b == b cond' a}.
inline Condition CommuteCondition(Condition cc) {
  switch (cc) {
    case below:
      return above;
    case above:
      return below;
    case above_equal:
      return below_equal;
    case below_equal:
      return above_equal;
    case less:
      return greater;
    case greater:
      return less;
    case greater_equal:
      return less_equal;
    case less_equal:
      return greater_equal;
    default:
      return cc;
  }
}

// Rounding mode.
enum RoundingMode {
  kRoundToNearest = 0x0,
  kRoundDown = 0x1,
  kRoundUp = 0x2,
  kRoundToZero = 0x3
};

}  // namespace jit
}  // namespace sling

#endif  // JIT_REGISTERS_H_
