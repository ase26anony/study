This appears to be code that decodes CPU cache configuration information, likely from CPUID instruction results on x86 processors. Here's what each part represents:

## Structure Fields
- `sizekb`: Cache size in kilobytes
- `assoc`: Associativity (number of ways)
- `line`: Cache line size in bytes

## Configuration Bytes
- `0x0a`: L1 cache - 8KB, 2-way set associative, 32-byte line size
- `0x0c`: L1 cache - 16KB, 4-way set associative, 32-byte line size  
- `0x86`: L2 cache - 512KB, 4-way set associative, 64-byte line size
- `0x87`: L2 cache - 1MB, 8-way set associative, 64-byte line size

## Context
This is typically part of CPU detection code that:
1. Executes the CPUID instruction with specific leaf functions
2. Gets cache configuration descriptors in registers (often in EAX/EBX/ECX/EDX)
3. Uses switch statements like this to decode the byte values into human-readable cache parameters

The pattern suggests this is for Intel processors, as AMD uses different encoding schemes for cache information. These byte values correspond to the "Cache Descriptors" returned by CPUID leaf functions like `CPUID.2` or `CPUID.4`.
