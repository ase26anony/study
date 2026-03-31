// Allocation
size_t data_length = 100; // Desired string length
struct tagged_string *ts = malloc(sizeof(struct tagged_string) + data_length + 1);

if (ts) {
    ts->length_bits = data_length * 8; // Store length in bits
    strcpy(ts->data, "Hello World");
    // ... use the string ...
    free(ts);
}
