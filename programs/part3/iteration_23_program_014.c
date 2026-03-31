#ifndef DWARF_ATTRIBUTES_H
#define DWARF_ATTRIBUTES_H

#ifdef __cplusplus
extern "C" {
#endif

/* For DW_AT_segment - address space qualifiers */
#ifdef __GNUC__
typedef int __attribute__((address_space(1))) *far_ptr_t;
#endif

/* For DW_AT_small - packed structure */
struct __attribute__((packed)) small_struct {
    unsigned int a : 4;
    unsigned int b : 4;
    unsigned int c : 8;
};

/* For DW_AT_string_length* attributes - Pascal-style string */
struct pstring {
    int length;
    char data[];
};

/* For DW_AT_picture_string - decimal type (COBOL/Ada style) */
#ifdef __GNUC__
typedef __attribute__((mode(SD))) _Decimal32 decimal32_t;
typedef __attribute__((mode(DD))) _Decimal64 decimal64_t;
#endif

/* For DW_AT_is_optional - function with optional parameter attribute */
#ifdef __GNUC__
int func_with_optional(int required, int optional __attribute__((optional)));
#endif

/* For DW_AT_prototyped - function prototype */
int prototyped_func(int a, double b, char c);

/* For DW_AT_lower_bound and DW_AT_ordering - array with non-zero lower bound */
#ifdef __GNUC__
/* GNU extension for array designators */
extern int array_with_bounds[10][1...4];
#endif

#ifdef __cplusplus
}
#endif

#endif /* DWARF_ATTRIBUTES_H */
