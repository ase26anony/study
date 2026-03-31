/* complex_types.h */
#ifndef COMPLEX_TYPES_H
#define COMPLEX_TYPES_H

#ifdef __GNUC__

/* For DW_AT_explicit and DW_AT_mutable */
struct TestClass {
    int normal_member;
    mutable int mutable_member;  /* DW_AT_mutable */
    
    TestClass() = default;
    explicit TestClass(int x) : normal_member(x) {}  /* DW_AT_explicit */
};

/* For DW_AT_picture_string (decimal type) */
typedef float __attribute__((mode(SD))) _Decimal32;
typedef float __attribute__((mode(DD))) _Decimal64;
typedef float __attribute__((mode(TD))) _Decimal128;

/* For DW_AT_small (packed structure) */
struct __attribute__((packed)) PackedStruct {
    char a;
    int b;
    char c;
};

/* For DW_AT_string_length and related attributes */
struct PascalString {
    int length;
    char data[];
};

/* For DW_AT_segment (address space) */
typedef int __attribute__((address_space(256))) far_int;

/* Function prototype for DW_AT_prototyped */
int prototype_func(int a, float b);  /* DW_AT_prototyped */

/* Optional parameter function (for DW_AT_is_optional) */
#ifdef __clang__
int optional_param_func(int a, ...) __attribute__((optional));
#endif

#endif /* __GNUC__ */

#endif /* COMPLEX_TYPES_H */
