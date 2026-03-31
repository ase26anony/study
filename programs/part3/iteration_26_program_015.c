#ifndef STRING_TYPES_H
#define STRING_TYPES_H

#ifdef __cplusplus
extern "C" {
#endif

/* Custom string type with explicit length field using bit-field */
struct custom_string {
    /* Length in bits (targets DW_AT_string_length_bit_size) */
    unsigned int length_bits : 12;  /* 12-bit length field */
    
    /* Length in bytes (targets DW_AT_string_length_byte_size) */
    unsigned char length_bytes;
    
    /* Actual string data as flexible array member */
    char data[];
} __attribute__((packed));

/* Function to create custom strings */
struct custom_string* create_custom_string(const char* str, unsigned int len);

/* Thread-local instance */
extern __thread struct custom_string* tl_string;

#ifdef __cplusplus
}
#endif

#endif /* STRING_TYPES_H */
