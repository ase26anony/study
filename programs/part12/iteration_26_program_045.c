This looks like a snippet from a CPU cache detection routine, likely from a tool like `cpuid` or a system information utility. Here's what each part does:

## Structure
- `level1` and `level2` are pointers to cache configuration structures
- Each structure likely contains fields like `sizekb` (size in KB), `assoc` (associativity), and `line` (cache line size in bytes)

## Case Breakdown:

### Case `0x0a`:
- **L1 cache**: 8KB, 2-way set associative, 32-byte cache lines
- This is a common L1 cache configuration for older Intel CPUs

### Case `0x2c`:
- **L1 cache**: 32KB, 8-way set associative, 64-byte cache lines
- This suggests a larger, more modern L1 cache configuration

### Case `0x49`:
- **L2 cache**: 4096KB (4MB), 16-way set associative, 64-byte cache lines
- The `if (xeon_mp)` check suggests this configuration might be different for Xeon MP processors
- If not a Xeon MP, it sets up a large 4MB L2 cache

## Context:
This code is parsing CPUID instruction results (specifically the CPUID leaf that returns cache information). The hex values (0x0a, 0x2c, 0x49) are cache descriptor bytes that Intel CPUs return to indicate specific cache configurations.

The full code would likely have many more cases covering different CPU models and their cache hierarchies (L1, L2, sometimes L3).
