#ifndef DWARF_ATTRIBUTES_H
#define DWARF_ATTRIBUTES_H

#ifdef __cplusplus
extern "C" {
#endif

/* For DW_AT_segment - address space qualifier */
#ifdef __GNUC__
typedef int __attribute__((address_space(256))) *far_pointer;
#endif

/* For DW_AT_string_length and related attributes */
typedef struct {
    int length;
    char data[256];
} pascal_string;

/* For DW_AT_small - packed structure */
#ifdef __GNUC__
struct __attribute__((packed)) small_struct {
    char a;
    int b;
    char c;
};
#endif

/* For DW_AT_picture_string - decimal type (simulated) */
#ifdef __GNUC__
typedef __attribute__((mode(SD))) int decimal_type;
#endif

/* Function prototypes for DW_AT_prototyped */
void function_with_prototype(int a, double b, char c);
int another_prototype(const char *str, ...);

#ifdef __cplusplus
}
#endif

#endif /* DWARF_ATTRIBUTES_H */
