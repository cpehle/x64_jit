# x64 jit assembler.

package(default_visibility = ["//visibility:public"])

licenses(["notice"]) # BSD

exports_files(["LICENSE"])

cc_library(
  name = "types",
  hdrs = ["types.h"],
  deps = [
  ],
)

cc_library(
  name = "memory",
  hdrs = ["memory.h"],
  deps = [
    ":types",
  ],
)

cc_library(
  name = "code",
  srcs = ["code.cc"],
  hdrs = ["code.h"],
  deps = [
    ":memory",
    ":types",
  ],
)

cc_library(
  name = "cpu",
  srcs = ["cpu.cc"],
  hdrs = ["cpu.h"],
  deps = [
  ],
)

cc_library(
  name = "assembler",
  srcs = ["assembler.cc"],
  hdrs = [
    "assembler.h",
    "instructions.h",
    "registers.h",
  ],
  deps = [
    ":code",
    ":cpu",
    ":memory",
    ":types",
  ],
)

