// Example of how this might be called
uint8_t cache_descriptor = get_cpuid_cache_descriptor();
switch(cache_descriptor) {
    case 0x0a: // ... set L1 cache
    case 0x86: // ... set L2 cache
    // etc.
}
