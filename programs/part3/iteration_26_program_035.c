#ifndef STRING_TYPES_H
#define STRING_TYPES_H

#include <stddef.h>
#include <stdint.h>

/* Custom string type with explicit length field using bit-field */
struct custom_string {
    /* Length field with specific bit size (12 bits) */
    unsigned int length : 12;
    /* Padding bits */
    unsigned int : 4;
    /* String data as flexible array member */
    char data[];
} __attribute__((packed));

/* Alternative using vendor extension if available */
#ifdef __GNUC__
/* Simulate string_length attribute using bit-field in separate struct */
struct string_with_bit_length {
    struct {
        unsigned int bit_length : 16;  /* Target DW_AT_string_length_bit_size */
        unsigned int byte_length : 8;  /* Target DW_AT_string_length_byte_size */
    } length_info;
    char* ptr;
} __attribute__((packed));
#endif

/* Thread-local instance */
extern __thread struct custom_string* tl_string;

#endif /* STRING_TYPES_H */
