#ifndef STRING_TYPES_H
#define STRING_TYPES_H

#ifdef __cplusplus
extern "C" {
#endif

/* Custom string type with explicit length field using bit-field */
struct custom_string {
    /* Length field with specific bit size (12 bits) */
    unsigned int length : 12;
    /* Padding bits */
    unsigned int : 4;
    /* Actual string data */
    char data[];
} __attribute__((packed));

/* Thread-local instance */
extern __thread struct custom_string *thread_local_string;

/* Function to create a custom string */
struct custom_string* create_custom_string(const char *str, unsigned int len);

/* Function to free custom string */
void free_custom_string(struct custom_string *s);

#ifdef __cplusplus
}
#endif

#endif /* STRING_TYPES_H */
