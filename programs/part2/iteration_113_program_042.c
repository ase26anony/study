// Example of what the calling code might look like
struct cache_config {
    int sizekb;
    int assoc;
    int line;
};

struct cache_config level1, level2;
uint8_t descriptor = get_cache_descriptor_byte();

switch(descriptor) {
    // ... cases like shown
}
