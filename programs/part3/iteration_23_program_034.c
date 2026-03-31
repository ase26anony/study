/* complex_types.h */
#ifndef COMPLEX_TYPES_H
#define COMPLEX_TYPES_H

#ifdef __GNUC__

/* For DW_AT_explicit and DW_AT_mutable */
struct TestClass {
    explicit TestClass(int x) : value(x) {}
    mutable int mutable_value;
    int value;
};

/* For DW_AT_picture_string (decimal type) */
typedef float __attribute__((mode(SD))) _Decimal32;
typedef float __attribute__((mode(DD))) _Decimal64;

/* For DW_AT_small (packed structure) */
struct __attribute__((packed)) SmallStruct {
    char a;
    int b;
    char c;
};

/* For DW_AT_string_length related attributes */
struct PascalString {
    int length;
    char data[256];
};

/* For DW_AT_segment (address space) */
typedef int __attribute__((address_space(256))) far_int;

#endif /* __GNUC__ */

#endif /* COMPLEX_TYPES_H */
