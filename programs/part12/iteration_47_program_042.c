// Allocation example:
struct tagged_string *create_string(int data_len) {
    struct tagged_string *ts = malloc(sizeof(struct tagged_string) + data_len + 1);
    ts->length_bits = data_len * 8; // Assuming 8 bits per char
    // Initialize data...
    return ts;
}
