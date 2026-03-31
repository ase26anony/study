#ifndef TYPES_H
#define TYPES_H

#ifdef __GNUC__

/* For DW_AT_explicit and DW_AT_mutable */
#ifdef __cplusplus
class ComplexClass {
private:
    int regular_member;
    mutable int mutable_member;  /* DW_AT_mutable */
public:
    explicit ComplexClass(int x) : regular_member(x), mutable_member(0) {}  /* DW_AT_explicit */
    void modify() const { mutable_member++; }
};
#endif

/* For DW_AT_picture_string - decimal type (COBOL/Ada style) */
typedef struct __attribute__((decimal(9,2))) {
    long long value;
} DecimalType;

/* For DW_AT_small - packed structure */
struct __attribute__((packed)) SmallStruct {
    char a;
    int b;
    char c;
};

/* For DW_AT_segment - pointer with address space */
typedef int __attribute__((address_space(1))) * FarPtr;

/* String type with length attributes */
struct PascalString {
    int length;  /* DW_AT_string_length */
    char data[];
};

/* Array with non-standard bounds (GNU extension) */
typedef int ArrayWithBounds[10][1...4];  /* DW_AT_lower_bound, DW_AT_ordering */

/* Function prototype for DW_AT_prototyped */
int prototyped_func(int a, double b, char c);  /* DW_AT_prototyped */

/* Optional parameter function (GNU extension) */
int optional_param_func(int a, ...) __attribute__((sentinel));  /* DW_AT_is_optional */

#endif /* __GNUC__ */

#endif /* TYPES_H */
