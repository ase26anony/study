// Allocate memory for structure + data
int needed_bits = 50;
int needed_bytes = (needed_bits + 7) / 8; // Round up to nearest byte
struct tagged_string *str = malloc(sizeof(struct tagged_string) + needed_bytes);

str->length_bits = needed_bits;
// Fill str->data with your content
