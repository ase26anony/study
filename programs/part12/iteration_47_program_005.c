// Allocate memory for struct + actual data size
size_t data_size = 100; // Desired string length
struct tagged_string *ts = malloc(sizeof(struct tagged_string) + data_size);

// Set the length
ts->length_bits = data_size * 8; // Convert bytes to bits

// Use the data
strcpy(ts->data, "Hello, World!");
