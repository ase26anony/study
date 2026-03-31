#ifndef STRING_TYPES_H
#define STRING_TYPES_H

#ifdef __cplusplus
extern "C" {
#endif

/* Custom string type with explicit length field using bit-field */
struct custom_string {
    /* Bit-field for length - targets DW_AT_string_length_bit_size */
    unsigned int length : 12;  /* 12-bit length field */
    
    /* Byte size attribute hint */
    unsigned int byte_size : 4;
    
    /* The actual string data */
    char data[];
} __attribute__((packed));

/* Thread-local instance */
extern __thread struct custom_string *thread_local_str;

/* Function prototypes */
struct custom_string* create_custom_string(const char *str, unsigned int len);
void free_custom_string(struct custom_string *s);

#ifdef __cplusplus
}
#endif

#endif /* STRING_TYPES_H */
