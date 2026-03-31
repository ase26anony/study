#ifndef STRING_TYPE_H
#define STRING_TYPE_H

#include <stddef.h>

/* Custom string type with explicit length attribute simulation */
#ifdef __GNUC__
/* Try to use vendor extension if available */
#define STRING_LENGTH_ATTR(N) __attribute__((string_length(N)))
#else
#define STRING_LENGTH_ATTR(N)
#endif

/* Structure to simulate string with explicit length bit size */
struct string_with_length {
    /* Length field using bit-field to force DW_AT_string_length_bit_size */
    unsigned int length : 12;  /* 12-bit length field */
    
    /* Data pointer */
    char *data;
    
    /* Padding to ensure alignment */
    unsigned char : 4;
} __attribute__((packed));

/* Thread-local instance */
extern __thread struct string_with_length tl_string;

/* Function to initialize string */
void init_string(struct string_with_length *str, const char *data);

#endif /* STRING_TYPE_H */
