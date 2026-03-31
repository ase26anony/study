#ifndef STRING_TYPES_H
#define STRING_TYPES_H

#ifdef __cplusplus
extern "C" {
#endif

/* Custom string type with explicit length attribute simulation */
struct custom_string {
    /* Length field with bit-field to potentially trigger DW_AT_string_length_bit_size */
    unsigned int length : 12;  /* 12-bit length field */
    
    /* Actual string data */
    char data[];
} __attribute__((packed));

/* Thread-local string type */
typedef struct custom_string* string_ptr_t;

/* Complex discriminated union targeting multiple DWARF attributes */
struct tagged_union {
    unsigned char tag;
    
    union {
        string_ptr_t str_ptr;
        int int_val;
        float float_val;
    } data __attribute__((packed));
} __attribute__((packed, aligned(2)));

/* Function prototype targeting DW_AT_prototyped */
void process_string_data(struct tagged_union* tu, int use_optional);

#ifdef __cplusplus
}
#endif

#endif /* STRING_TYPES_H */
