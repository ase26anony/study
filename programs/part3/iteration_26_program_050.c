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
};

/* Thread-local instance - targets DW_AT_threads_scaled */
extern __thread struct custom_string *thread_local_string;

/* Function to create a custom string */
struct custom_string* create_custom_string(const char *str, unsigned int len);

/* Function to free custom string */
void free_custom_string(struct custom_string *s);

#ifdef __cplusplus
}
#endif

#endif /* STRING_LENGTH_H */
