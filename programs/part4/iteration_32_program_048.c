This appears to be part of a CPU cache detection routine, likely from a system information tool or CPUID decoder. Here's what I can deduce:

## Context
This is parsing CPUID cache information (specifically the `CPUID.2` or `CPUID.4` leaf) to determine cache characteristics. The hex values (0x0a, 0x0c, 0x21, etc.) are cache descriptor bytes from CPUID.

## Structure
- `level1` and `level2` point to L1 and L2 cache configuration structures
- Each case handles a specific cache descriptor byte
- Fields being set:
  - `sizekb`: Cache size in kilobytes
  - `assoc`: Associativity (number of ways)
  - `line`: Cache line size in bytes

## Examples from the code:

**L1 Cache:**
- `0x0a`: 8KB, 2-way associative, 32-byte line
- `0x0c`: 16KB, 4-way associative, 32-byte line

**L2 Cache:**
- `0x21`: 256KB, 8-way associative, 64-byte line  
- `0x24`: 1MB, 16-way associative, 64-byte line
- `0x49`: 4MB, 16-way associative, 64-byte line (with Xeon MP check)

## Special Case
The `0x49` descriptor has a special check for Xeon MP processors - if it's a Xeon MP, it breaks without setting the L2 cache, suggesting this descriptor might mean something different for those CPUs.

This is typical of low-level CPU detection code found in tools like `cpuid`, `dmidecode`, or kernel initialization code.
