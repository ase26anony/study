This appears to be part of a CPU cache detection routine, likely from a system information tool or CPUID decoder. Here's what I can tell:

## Structure
- **Case statements** are handling CPUID leaf 2 (cache descriptor) values
- **level1** and **level2** are structures representing L1 and L2 cache configurations
- Each case corresponds to a specific cache configuration byte from CPUID

## Cache Parameters Being Set:
1. **sizekb** - Cache size in kilobytes
2. **assoc** - Associativity (number of ways)
3. **line** - Cache line size in bytes

## Examples:
- `0x0a`: L1 cache, 8KB, 2-way associative, 32-byte lines
- `0x0c`: L1 cache, 16KB, 4-way associative, 32-byte lines  
- `0x21`: L2 cache, 256KB, 8-way associative, 64-byte lines
- `0x24`: L2 cache, 1MB, 16-way associative, 64-byte lines
- `0x49`: L2 cache, 4MB, 16-way associative, 64-byte lines (except for Xeon MP processors)

## Interesting Detail:
The `0x49` case has a special check: `if (xeon_mp) break;` - meaning this 4MB L2 cache configuration doesn't apply to Xeon MP processors, suggesting different cache configurations for server vs desktop/workstation CPUs.

This code is parsing CPUID information to determine the exact cache hierarchy of Intel processors, which is crucial for performance optimization and system reporting.
