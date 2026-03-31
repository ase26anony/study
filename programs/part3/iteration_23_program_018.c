/* complex_types.h */
#ifndef COMPLEX_TYPES_H
#define COMPLEX_TYPES_H

#ifdef __cplusplus
#include <cstdio>
extern "C" {
#else
#include <stdio.h>
#endif

/* For DW_AT_explicit and DW_AT_mutable */
#ifdef __cplusplus
class TestClass {
private:
    int regular_member;
public:
    mutable int mutable_member;  /* Should trigger DW_AT_mutable */
    
    /* Explicit constructor - should trigger DW_AT_explicit */
    explicit TestClass(int x) : regular_member(x), mutable_member(x) {}
    
    void print() const {
        printf("Value: %d\n", regular_member);
    }
};
#endif

/* For DW_AT_picture_string (decimal types) */
typedef struct {
    long long value;
} __attribute__((decimal(9,2))) DecimalType;  /* 9 digits, 2 decimal places */

/* For DW_AT_small (packed structure) */
struct __attribute__((packed)) SmallStruct {
    char a;
    int b;
    char c;
};

/* For DW_AT_segment (address space pointers) */
#ifdef __GNUC__
typedef int __attribute__((address_space(256))) *FarPointer;
#endif

/* For string length attributes */
typedef struct {
    unsigned int length;
    char data[256];
} PascalString;

#ifdef __cplusplus
}
#endif

#endif /* COMPLEX_TYPES_H */
