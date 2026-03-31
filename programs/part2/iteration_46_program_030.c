// The struct layout would be:
// - id: 4 bytes (typical int size)
// - data: 16 bytes (128 bits = 16 bytes)
// Total size: 20 bytes (plus potential padding)

struct Message msg;
printf("Size of Message: %zu\n", sizeof(struct Message)); // Likely 20 or 24 bytes
printf("Size of encrypted_string: %zu\n", sizeof(encrypted_string)); // 16 bytes
