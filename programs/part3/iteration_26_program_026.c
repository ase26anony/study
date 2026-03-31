#ifndef STRING_TYPES_H
#define STRING_TYPES_H

#include <stddef.h>

/* Custom string type with explicit length field using bit-field */
struct custom_string {
    /* Length field with specific bit size - targets DW_AT_string_length_bit_size */
    unsigned int length : 12;  /* 12-bit length field */
    
    /* Data follows - flexible array member */
    char data[];
};

/* Thread-local instance */
extern __thread struct custom_string* thread_local_string;

/* Function to create custom string */
struct custom_string* create_custom_string(const char* str, unsigned int len);

/* Function to get string length */
unsigned int get_string_length(const struct custom_string* s);

#endif /* STRING_TYPES_H */
