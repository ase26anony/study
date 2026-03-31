// Example of how this might be used
void decode_cache_descriptor(uint8_t descriptor, CacheInfo *level1, CacheInfo *level2) {
    switch(descriptor) {
        case 0x0a:
            level1->sizekb = 8; level1->assoc = 2; level1->line = 32;
            break;
        // ... more cases
        case 0x86:
            level2->sizekb = 512; level2->assoc = 4; level2->line = 64;
            break;
        // ... etc
    }
}
