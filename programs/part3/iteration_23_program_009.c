#ifndef DWARF_TYPES_H
#define DWARF_TYPES_H

#ifdef __cplusplus
extern "C" {
#endif

/* For DW_AT_picture_string - decimal type (COBOL/Ada style) */
#ifdef __GNUC__
typedef int __attribute__((decimal(9,2))) decimal_type;
#else
typedef int decimal_type;
#endif

/* For DW_AT_string_length attributes - Pascal-style string */
struct pascal_string {
    int length;
    char data[256];
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

/* For DW_AT_segment - pointer with address space */
#ifdef __GNUC__
typedef int __attribute__((address_space(1))) *segment_ptr;
#else
typedef int *segment_ptr;
#endif

#ifdef __cplusplus
}
#endif

#endif /* DWARF_TYPES_H */
