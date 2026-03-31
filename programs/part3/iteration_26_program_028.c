#ifndef STRING_LENGTH_H
#define STRING_LENGTH_H

#ifdef __cplusplus
extern "C" {
#endif

/* Custom string type with explicit length field using bit-field */
struct custom_string {
    /* Length field with specific bit size - targets DW_AT_string_length_bit_size */
    unsigned int length : 12;  /* 12-bit length field */
    
    /* Actual string data */
    char data[];
} __attribute__((packed));

/* Function to create and initialize custom strings */
struct custom_string* create_custom_string(const char* str);

#ifdef __cplusplus
}
#endif

#endif /* STRING_LENGTH_H */
