struct tagged_string *create_string(int length_bits) {
    size_t bytes_needed = sizeof(struct tagged_string) + 
                          ((length_bits + 7) / 8); // Convert bits to bytes
    struct tagged_string *ts = malloc(bytes_needed);
    if (ts) {
        ts->length_bits = length_bits;
    }
    return ts;
}
