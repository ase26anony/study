#ifndef DWARF_ATTRIBUTES_H
#define DWARF_ATTRIBUTES_H

#ifdef __cplusplus
extern "C" {
#endif

/* For DW_AT_segment - address space qualifiers */
#ifdef __GNUC__
typedef int __attribute__((address_space(1))) *far_ptr_t;
#endif

/* For DW_AT_string_length attributes - Pascal-style string */
struct pstring {
    int length;
    char data[];
};

/* For DW_AT_small - packed structure */
struct __attribute__((packed)) small_struct {
    char a;
    int b;
    char c;
};

/* For DW_AT_picture_string - decimal type (COBOL/Ada style) */
#ifdef __GNUC__
typedef int __attribute__((mode(SD))) decimal_type;
#endif

/* Function prototypes for DW_AT_prototyped */
void func_with_prototype(int a, float b, char c);

#ifdef __cplusplus
}
#endif

#endif /* DWARF_ATTRIBUTES_H */
