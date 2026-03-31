#ifndef STRING_TYPES_H
#define STRING_TYPES_H

#ifdef __cplusplus
extern "C" {
#endif

/* Custom string type with explicit length field using bit-field */
struct custom_string {
    /* Length in bits (targets DW_AT_string_length_bit_size) */
    unsigned int length_bits : 12;
    
    /* Length in bytes (targets DW_AT_string_length_byte_size) */
    unsigned int length_bytes : 12;
    
    /* Padding to align */
    unsigned int : 8;
    
    /* Flexible array member for actual string data */
    char data[];
} __attribute__((packed));

/* Thread-local instance */
extern __thread struct custom_string *thread_local_string;

/* Function to create a custom string */
struct custom_string *create_custom_string(const char *str, unsigned int len);

/* Function to free custom string */
void free_custom_string(struct custom_string *s);

#ifdef __cplusplus
}
#endif

#endif /* STRING_TYPES_H */
