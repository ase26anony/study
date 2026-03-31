This looks like a snippet of code from a CPU cache detection routine, likely from a tool like `cpuid` or a system information utility. The code is interpreting CPUID leaf 2 (cache descriptor) values to determine cache parameters.

Here's what each part does:

## Structure
- `level1`, `level2` are pointers to cache configuration structures
- Each structure likely contains fields like `sizekb` (size in KB), `assoc` (associativity), `line` (cache line size in bytes)

## Case Breakdown:

**Case 0x0a:**
- Sets L1 cache to: 8KB size, 2-way associative, 32-byte line size
- This is a typical L1 data cache configuration for older Intel CPUs

**Case 0x2c:**
- Sets L1 cache to: 32KB size, 8-way associative, 64-byte line size
- Common for modern L1 data caches

**Case 0x49:**
- Special case for L2 cache: 4MB size, 16-way associative, 64-byte line size
- Has a condition: `if (xeon_mp) break;` - skips this configuration for Xeon MP processors
- This suggests the same cache descriptor byte (0x49) might mean different things for different CPU families

## Context:
This is decoding CPUID leaf 2 values, where each byte returned by CPUID represents a cache descriptor. The values 0x0a, 0x2c, 0x49 are hexadecimal cache descriptor bytes that Intel documents in their CPUID specification.

The full code would have many more cases covering different cache configurations for various Intel CPU models.
