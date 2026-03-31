#ifndef TYPES_H
#define TYPES_H

#ifdef __GNUC__

/* For DW_AT_explicit and DW_AT_mutable */
class ComplexClass {
private:
    int regular_member;
    mutable int mutable_member;  /* DW_AT_mutable */
    
public:
    explicit ComplexClass(int x) : regular_member(x), mutable_member(0) {}  /* DW_AT_explicit */
    void modify() const { mutable_member++; }
};

/* For DW_AT_small - packed structure */
struct __attribute__((packed)) PackedStruct {
    char a;
    int b;
    char c;
};

/* For DW_AT_picture_string - decimal type (COBOL/Ada style) */
typedef struct __attribute__((decimal(9,2))) {
    unsigned char digits[5];
} DecimalType;

/* For DW_AT_segment - pointer with address space */
typedef int __attribute__((address_space(1))) * FarPointer;

/* For string attributes - Pascal-style string */
typedef struct {
    int length;  /* DW_AT_string_length */
    char data[];
} PascalString;

/* Function prototype for DW_AT_prototyped */
int prototype_func(int a, double b);  /* DW_AT_prototyped */

/* Optional parameter function (for DW_AT_is_optional) */
#ifdef __clang__
int optional_param_func(int a, ...) __attribute__((optional));
#endif

#endif /* __GNUC__ */

#endif /* TYPES_H */
