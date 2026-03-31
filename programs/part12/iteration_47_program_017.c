#include <stdlib.h>
#include <string.h>

struct tagged_string* create_tagged_string(const char* str) {
    size_t len = strlen(str);
    // Allocate space for struct + string + null terminator
    struct tagged_string* ts = malloc(sizeof(struct tagged_string) + len + 1);
    
    if (ts) {
        ts->length_bits = len * 8; // Assuming 8-bit characters
        memcpy(ts->data, str, len + 1); // Copy string with null terminator
    }
    
    return ts;
}

// Usage
struct tagged_string* my_string = create_tagged_string("Hello");
printf("Length in bits: %d\n", my_string->length_bits);
printf("Data: %s\n", my_string->data);
free(my_string);
