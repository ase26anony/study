#ifndef STRING_LENGTH_H
#define STRING_LENGTH_H

#include <stddef.h>
#include <stdint.h>

/* Custom string type with explicit length field using bit-field */
struct custom_string {
    /* Length field with specific bit size (12 bits) */
    unsigned int length : 12;
    /* Padding to ensure alignment */
    unsigned int : 4;
    /* Flexible array member for string data */
    char data[];
} __attribute__((packed));

/* Alternative approach using vendor extension if available */
#ifdef __GNUC__
/* Simulate string_length attribute using bit-field in a union */
typedef union {
    struct {
        unsigned int len_bits : 16;  /* Target DW_AT_string_length_bit_size */
        unsigned char len_bytes;     /* Target DW_AT_string_length_byte_size */
    } length_info;
    size_t full_length;
} string_length_attr __attribute__((packed));
#endif

/* Function to create custom string */
struct custom_string* create_custom_string(const char* str, unsigned int len);

#endif /* STRING_LENGTH_H */
