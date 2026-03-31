This looks like a snippet of code from a CPU cache detection routine, likely from a tool like `cpuid` or a system information utility. Here's what's happening:

## What this code does:
It's parsing CPUID cache information to determine cache sizes and properties. The hex values (0x0a, 0x2c, 0x49) are cache descriptor values returned by the CPUID instruction.

## Breakdown of the cases:

1. **Case 0x0a**:
   - Sets L1 cache to: 8KB size, 2-way associative, 32-byte line size

2. **Case 0x2c**:
   - Sets L1 cache to: 32KB size, 8-way associative, 64-byte line size

3. **Case 0x3c**:
   - Special case for Xeon MP processors
   - If not Xeon MP, sets L2 cache to: 4096KB (4MB), 16-way associative, 64-byte line size

## Structure pointers:
- `level1`: Points to L1 cache configuration structure
- `level2`: Points to L2 cache configuration structure
- `xeon_mp`: Boolean flag indicating if the CPU is a Xeon MP (Multi-Processor) variant

## Context:
This is typically part of CPU feature detection where the CPUID instruction returns cache parameters in specific registers, and these hex values are standardized cache descriptor codes defined by Intel/AMD. Each code corresponds to a specific cache configuration.

The code appears to be handling special cases where certain cache configurations might differ between processor models (like the Xeon MP exception in case 0x49).
