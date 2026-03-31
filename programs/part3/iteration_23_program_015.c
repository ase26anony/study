#ifndef DWARF_ATTRIBUTES_H
#define DWARF_ATTRIBUTES_H

#ifdef __cplusplus
extern "C" {
#endif

/* For DW_AT_segment - address space qualifier */
#ifdef __GNUC__
typedef int __attribute__((address_space(256))) *far_pointer;
#else
typedef int *far_pointer;
#endif

/* For DW_AT_string_length and related attributes */
struct pascal_string {
    int length;
    char data[100];
};

/* For DW_AT_small - packed structure */
#ifdef __GNUC__
struct __attribute__((packed)) small_struct {
    char a;
    int b;
    char c;
};
#else
#pragma pack(push, 1)
struct small_struct {
    char a;
    int b;
    char c;
};
#pragma pack(pop)
#endif

/* For DW_AT_picture_string - decimal type (COBOL/Ada style) */
#ifdef __GNUC__
typedef int __attribute__((decimal)) decimal_type;
#else
typedef int decimal_type;
#endif

/* Function prototypes for DW_AT_prototyped */
void prototyped_function(int a, double b, char c);
int optional_param_function(int required, int optional);

#ifdef __cplusplus
}
#endif

#endif /* DWARF_ATTRIBUTES_H */
