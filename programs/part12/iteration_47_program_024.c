// Allocate with space for 10 bytes of data
struct tagged_string *ts = malloc(sizeof(struct tagged_string) + 10);
ts->length_bits = 80; // 10 bytes * 8 bits = 80 bits

// Or for partial bytes
ts->length_bits = 75; // 9 full bytes + 3 bits
