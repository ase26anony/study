This appears to be part of a CPU cache detection routine, likely from a system information tool or CPUID decoder. Here's what I can tell:

## Structure
- **`level1`** and **`level2`** are pointers to cache configuration structures
- Each structure has fields: `sizekb` (cache size in KB), `assoc` (associativity), `line` (cache line size in bytes)
- The hex values (0x0a, 0x2c, 0x49) are CPUID cache descriptor values

## Specific Cases Shown:

1. **Case 0x0a**: L1 cache
   - Size: 8 KB
   - Associativity: 2-way
   - Line size: 32 bytes

2. **Case 0x2c**: L1 cache  
   - Size: 32 KB
   - Associativity: 8-way
   - Line size: 64 bytes

3. **Case 0x49**: L2 cache (with special condition)
   - Size: 4096 KB (4 MB)
   - Associativity: 16-way
   - Line size: 64 bytes
   - **Condition**: Only applies if `xeon_mp` is false (skipped for Xeon MP processors)

## Context:
This is decoding Intel CPUID leaf 2 or leaf 4 cache information, where specific byte values correspond to predefined cache configurations according to Intel's documentation.

The `xeon_mp` check suggests this code handles different processor families where the same cache descriptor might mean different things depending on the CPU model.
