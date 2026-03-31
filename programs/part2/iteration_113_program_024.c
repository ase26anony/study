// Typical structure
struct cache_info {
    int sizekb;
    int assoc;
    int line;
};

// Function to decode cache configuration
void decode_cache_config(uint8_t config_byte, struct cache_info *cache) {
    switch(config_byte) {
        case 0x0a:
            cache->sizekb = 8; cache->assoc = 2; cache->line = 32;
            break;
        case 0x0c:
            cache->sizekb = 16; cache->assoc = 4; cache->line = 32;
            break;
        // ... more L1 cases
        case 0x86:
            cache->sizekb = 512; cache->assoc = 4; cache->line = 64;
            break;
        case 0x87:
            cache->sizekb = 1024; cache->assoc = 8; cache->line = 64;
            break;
        // ... more L2/L3 cases
        default:
            // Unknown configuration
            break;
    }
}
