#ifndef DWARF_TEST_H
#define DWARF_TEST_H

#include <optional>
#include <cstddef>

#ifdef __cplusplus
extern "C" {
#endif

/* For DW_AT_prototyped - old K&R style prototype */
int old_style_proto();  /* No parameter list - forces DW_AT_prototyped */

/* For DW_AT_string_length attributes */
struct string_with_length {
    size_t length;
    char data[];  /* Flexible array member */
};

/* For DW_AT_picture_string - simulate COBOL-like type */
typedef char PICTURE[20];  /* Picture string type */

#ifdef __cplusplus
}
#endif

/* C++ specific constructs */
#ifdef __cplusplus

/* For DW_AT_explicit */
class ExplicitClass {
    int value;
public:
    explicit ExplicitClass(int v) : value(v) {}
    int get() const { return value; }
};

/* For DW_AT_mutable */
class ClassWithMutable {
    int normal;
    mutable int mutable_member;  /* This should trigger DW_AT_mutable */
public:
    ClassWithMutable() : normal(0), mutable_member(0) {}
    void modify() const { mutable_member = 42; }  /* Can modify mutable in const */
};

/* For DW_AT_ordering */
enum OrderedEnum {
    FIRST = 10,
    SECOND = 5,    /* Out of order */
    THIRD = 15,
    FOURTH = 0     /* Not in declaration order */
};

/* For DW_AT_small - packed struct with bitfields */
struct __attribute__((packed)) SmallPackedStruct {
    unsigned char tiny : 3;    /* 3-bit field */
    unsigned char small : 5;   /* 5-bit field */
    signed char small_signed : 4;  /* Small signed type */
    int normal_int;
};

/* Template to force emission in multiple contexts */
template<typename T>
class Container {
    T value;
public:
    Container(T v) : value(v) {}
    T get() const { return value; }
};

#endif /* __cplusplus */

#endif /* DWARF_TEST_H */
