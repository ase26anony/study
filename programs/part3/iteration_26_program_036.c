#ifndef STRING_TYPES_H
#define STRING_TYPES_H

#ifdef __cplusplus
extern "C" {
#endif

/* Custom string type with explicit length attribute simulation */
struct custom_string {
    /* Using bit-field to force DW_AT_string_length_bit_size */
    unsigned int length : 12;  /* 12-bit length field */
    
    /* Flexible array member for the string data */
    char data[];
} __attribute__((packed));

/* Thread-local storage for DW_AT_threads_scaled */
extern __thread struct custom_string *thread_local_string;

/* Function prototypes for DW_AT_prototyped */
struct custom_string *create_string(const char *str, unsigned int len);
void free_string(struct custom_string *s);

#ifdef __cplusplus
}
#endif

#endif /* STRING_TYPES_H */
