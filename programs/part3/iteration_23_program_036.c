/* complex_types.h */
#ifndef COMPLEX_TYPES_H
#define COMPLEX_TYPES_H

#ifdef __GNUC__

/* For DW_AT_explicit and DW_AT_mutable */
class ComplexClass {
private:
    int regular_member;
public:
    mutable int mutable_member;  /* Should trigger DW_AT_mutable */
    
    /* Explicit constructor - should trigger DW_AT_explicit */
    explicit ComplexClass(int x) : regular_member(x), mutable_member(0) {}
    
    void set_mutable(int val) const { mutable_member = val; }
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

#endif /* __GNUC__ */

#endif /* COMPLEX_TYPES_H */
