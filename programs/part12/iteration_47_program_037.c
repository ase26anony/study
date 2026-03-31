// Allocation example:
struct tagged_string *create_string(int data_length) {
    struct tagged_string *ts = malloc(sizeof(struct tagged_string) + data_length + 1);
    ts->length_bits = data_length * 8; // Assuming 8 bits per char
    // Initialize data...
    return ts;
}
