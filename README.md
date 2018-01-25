# x64_jit

This is a standalone version of a x86_64 macro assembler that originated apparently at Sun and is used in the v8 javascript engine. 
The code was found and copied from Google's sling project, the license covering the code can be found in the jit/ directory. 

## Building

```
bazel build //jit:assembler
```
