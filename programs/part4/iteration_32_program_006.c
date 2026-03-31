This appears to be code from a CPU cache detection routine, likely from a system information tool or CPUID decoder. Here's what each part does:

## Structure
- `level1` and `level2` are pointers to cache configuration structures
- Each case corresponds to a CPUID cache descriptor value
- The code sets cache parameters based on these encoded values

## Cache Parameters Being Set:
- `sizekb`: Cache size in kilobytes
- `assoc`: Associativity (number of ways)
- `line`: Cache line size in bytes

## Specific Cases Shown:

**L1 Cache:**
- `0x0a`: 8KB, 2-way associative, 32-byte line
- `0x0c`: 16KB, 4-way associative, 32-byte line

**L2 Cache:**
- `0x21`: 256KB, 8-way associative, 64-byte line  
- `0x24`: 1MB, 16-way associative, 64-byte line
- `0x49`: 4MB, 16-way associative, 64-byte line (with Xeon MP check)

## Notable Feature:
The `0x49` case includes a check for `xeon_mp` (Xeon Multi-Processor) - if true, it breaks without setting the L2 cache. This suggests some Xeon MP processors might report this cache descriptor but have different actual cache characteristics.

This is typical of CPU cache detection code found in tools like `cpuid`, `dmidecode`, or system information libraries that decode CPUID information to determine cache hierarchy.
