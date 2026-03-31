This appears to be code from a CPU cache detection routine, likely from a system information tool or CPUID decoder. Here's what each part does:

## Structure Explanation
- `level1` and `level2` are pointers to cache configuration structures
- Each structure has:
  - `sizekb`: Cache size in kilobytes
  - `assoc`: Associativity (number of ways)
  - `line`: Cache line size in bytes

## Hex Code Meanings
The hex values (0x0a, 0x0c, 0x21, etc.) are CPUID cache descriptor values that identify specific cache configurations.

## Examples Decoded:

**L1 Cache:**
- `0x0a`: 8KB, 2-way associative, 32-byte line size
- `0x0c`: 16KB, 4-way associative, 32-byte line size

**L2 Cache:**
- `0x21`: 256KB, 8-way associative, 64-byte line size  
- `0x24`: 1MB, 16-way associative, 64-byte line size
- `0x49`: 4MB, 16-way associative, 64-byte line size (with Xeon MP check)

## Special Case
The `0x49` case includes a check for `xeon_mp` (Xeon Multi-Processor) - if true, it breaks without setting the L2 cache values, suggesting this configuration might be invalid or handled differently for Xeon MP processors.

This code is mapping CPUID cache identification values to their actual cache characteristics for display or configuration purposes.
