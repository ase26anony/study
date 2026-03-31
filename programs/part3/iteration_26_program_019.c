#ifndef STRING_TYPES_H
#define STRING_TYPES_H

#include <stddef.h>

/* Custom string type with explicit length field using bit-field */
struct custom_string {
    /* Bit-field for length - targets DW_AT_string_length_bit_size */
    unsigned int length : 12;  /* 12-bit length field */
    
    /* Byte size attribute - targets DW_AT_string_length_byte_size */
    unsigned char byte_size;
    
    /* Flexible array member for actual string data */
    char data[];
} __attribute__((packed));

/* Thread-local instance */
extern __thread struct custom_string *thread_local_string;

/* Function to create a custom string */
struct custom_string* create_custom_string(const char *str, unsigned int len);

/* Function to free custom string */
void free_custom_string(struct custom_string *cs);

#endif /* STRING_TYPES_H */
