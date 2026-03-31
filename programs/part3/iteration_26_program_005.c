#ifndef CUSTOM_TYPES_H
#define CUSTOM_TYPES_H

#ifdef __cplusplus
extern "C" {
#endif

/* Custom string type with explicit length field using bit-field */
struct custom_string {
    /* Length in bits (targets DW_AT_string_length_bit_size) */
    unsigned int length_bits : 12;
    /* Length in bytes (targets DW_AT_string_length_byte_size) */
    unsigned int length_bytes : 12;
    /* Actual string data */
    char data[];
} __attribute__((packed));

/* Complex discriminated union with packed attribute */
struct tagged_union {
    enum { INT_VARIANT, STR_VARIANT, PTR_VARIANT } tag;
    
    union {
        int int_value;
        struct custom_string* str_ptr;
        void* generic_ptr;
    } value;
    
    /* Packed to affect alignment/dwarf attributes */
} __attribute__((packed, aligned(2)));

/* Function prototype for cross-TU usage */
void process_union(struct tagged_union* tu);

#ifdef __cplusplus
}
#endif

#endif /* CUSTOM_TYPES_H */
