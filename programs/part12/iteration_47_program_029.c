// Allocate space for 40 bits of data (5 bytes)
int bits_needed = 40;
int bytes_needed = (bits_needed + 7) / 8; // Ceiling division

struct tagged_string *str = malloc(sizeof(struct tagged_string) + bytes_needed);
str->length_bits = bits_needed;
// Now str->data can hold 40 bits (5 bytes) of data
