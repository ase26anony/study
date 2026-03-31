#include <stdlib.h>
#include <string.h>

struct tagged_string* create_tagged_string(const char* str, int length_bits) {
    size_t data_len = strlen(str) + 1; // +1 for null terminator
    struct tagged_string* ts = malloc(sizeof(struct tagged_string) + data_len);
    
    if (ts) {
        ts->length_bits = length_bits;
        memcpy(ts->data, str, data_len);
    }
    
    return ts;
}
