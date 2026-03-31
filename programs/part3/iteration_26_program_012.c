#ifndef STRING_TYPE_H
#define STRING_TYPE_H

#ifdef __cplusplus
extern "C" {
#endif

/* Custom string type with explicit length field using bit-field */
struct custom_string {
    /* Length field with specific bit size (12 bits) */
    unsigned int length : 12;
    /* Padding to ensure alignment */
    unsigned int : 4;
    /* Flexible array member for the string data */
    char data[];
} __attribute__((packed));

/* Alternative approach using attribute if supported */
#ifdef __GNUC__
/* Try to use string_length attribute if compiler supports it */
typedef struct {
    unsigned int length;
    char data[0];
} __attribute__((string_length(length))) string_with_length;
#endif

/* Function prototypes */
struct custom_string* create_string(const char* src);
unsigned int string_hash(const struct custom_string* s);

#ifdef __cplusplus
}
#endif

#endif /* STRING_TYPE_H */
