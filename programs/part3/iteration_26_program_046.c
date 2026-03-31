#ifndef STRING_TYPES_H
#define STRING_TYPES_H

#include <stddef.h>

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
/* Try to use vendor extension for string length attributes */
typedef struct {
    unsigned int length;
    char data[];
} __attribute__((string_length(12))) string_with_length;
#endif

/* Function prototypes that will generate DW_AT_prototyped */
struct custom_string* create_string(const char* src);
void free_string(struct custom_string* str);
size_t string_byte_size(const struct custom_string* str);

#ifdef __cplusplus
}
#endif

#endif /* STRING_TYPES_H */
