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

/* Alternative approach using GNU extension if available */
#ifdef __GNUC__
struct gnu_string {
    int len __attribute__((string_length(12)));
    char *data;
};
#endif

/* Function prototypes */
void process_string(const struct custom_string *str);

#ifdef __cplusplus
}
#endif

#endif /* STRING_TYPE_H */
