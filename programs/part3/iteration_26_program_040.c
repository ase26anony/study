#ifndef STRING_TYPES_H
#define STRING_TYPES_H

#ifdef __cplusplus
extern "C" {
#endif

/* Custom string type with explicit length field using bit-field */
struct custom_string {
    /* Bit-field for length - should trigger DW_AT_string_length_bit_size */
    unsigned int length : 12;  /* 12-bit length field */
    
    /* Byte size attribute hint */
    unsigned int byte_size : 3; /* 3 bits for byte size info */
    
    /* Flexible array member for actual string data */
    char data[];
} __attribute__((packed));

/* Alternative approach using GNU extension if available */
#ifdef __GNUC__
struct gnu_string {
    unsigned long length __attribute__((string_length(8))); /* Hypothetical attribute */
    char *data;
} __attribute__((packed));
#endif

/* Thread-local instance declaration */
extern __thread struct custom_string *thread_local_string;

#ifdef __cplusplus
}
#endif

#endif /* STRING_TYPES_H */
