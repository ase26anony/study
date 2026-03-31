// Allocation example:
struct tagged_string *create_string(int length_bits, size_t data_size) {
    struct tagged_string *ts = malloc(sizeof(struct tagged_string) + data_size);
    ts->length_bits = length_bits;
    // data is already allocated as part of the same block
    return ts;
}
