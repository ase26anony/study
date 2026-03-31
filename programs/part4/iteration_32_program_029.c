This appears to be code from a CPU cache detection routine, likely from a system information tool or CPUID decoder. Here's what each part does:

## Structure
- `level1` and `level2` are pointers to cache configuration structures
- Each case corresponds to a CPUID cache descriptor value (hexadecimal)
- The code sets cache parameters based on the CPU's reported cache configuration

## Cache Parameters Being Set:
1. **`sizekb`** - Cache size in kilobytes
2. **`assoc`** - Associativity (number of ways)
3. **`line`** - Cache line size in bytes

## Specific Cases Shown:

**L1 Cache:**
- `0x0a`: 8KB, 2-way associative, 32-byte line
- `0x0c`: 16KB, 4-way associative, 32-byte line

**L2 Cache:**
- `0x21`: 256KB, 8-way associative, 64-byte line  
- `0x24`: 1024KB (1MB), 16-way associative, 64-byte line
- `0x49`: 4096KB (4MB), 16-way associative, 64-byte line (except for Xeon MP processors)

## Special Handling:
The `0x49` case includes a check for `xeon_mp` (Xeon Multi-Processor) - if true, it breaks without setting the L2 cache, suggesting this configuration doesn't apply to Xeon MP CPUs.

This is typical code for parsing CPUID leaf 2 (cache descriptors) or leaf 4 (deterministic cache parameters) information to determine the CPU's cache hierarchy.
