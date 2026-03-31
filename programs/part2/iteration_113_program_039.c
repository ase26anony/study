// This would be part of a larger switch statement parsing CPUID leaf 2
// or similar cache identification mechanism
uint8_t descriptor = get_cache_descriptor();
switch(descriptor) {
    // ... cases like shown above
}
