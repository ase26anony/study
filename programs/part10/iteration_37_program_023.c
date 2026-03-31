#ifndef DWARF_TEST_H
#define DWARF_TEST_H

#include <optional>
#include <cstddef>

#ifdef __cplusplus
extern "C" {
#endif

/* For DW_AT_prototyped - old K&R style prototype */
int old_style_proto(); /* No parameter list - K&R style */

/* For DW_AT_picture_string - simulate COBOL-like type */
typedef char PICTURE[20];
struct CobolLike {
    PICTURE picture_field;
};

/* For DW_AT_string_length attributes */
struct StringWithLength {
    size_t length;
    char data[]; /* Flexible array member */
};

/* For DW_AT_segment */
#ifdef __GNUC__
#define SECTION_ATTR __attribute__((section(".mysection")))
#else
#define SECTION_ATTR
#endif

extern SECTION_ATTR int segment_var;

/* For DW_AT_threads_scaled */
#ifdef _OPENMP
extern int omp_thread_array[10];
#endif

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
    mutable int mutable_member;
public:
    ClassWithMutable() : normal(0), mutable_member(0) {}
    void const_method() const {
        /* Can modify mutable member even in const method */
        mutable_member = 42;
    }
    int get_mutable() const { return mutable_member; }
};

/* For DW_AT_is_optional */
template<typename T>
struct MaybeOptional {
    std::optional<T> opt_value;
    T* maybe_null = nullptr;
    
    MaybeOptional() = default;
    explicit MaybeOptional(T val) : opt_value(val) {}
    
    T get_or_default(T def) {
        return opt_value.value_or(def);
    }
};

/* For DW_AT_ordering */
enum OrderedEnum {
    ENUM_FIRST = 10,
    ENUM_SECOND = 5,
    ENUM_THIRD = 20,
    ENUM_FOURTH = 15
};

/* For DW_AT_small */
struct __attribute__((packed)) PackedSmall {
    unsigned char small_char : 4;
    unsigned char tiny_bool : 1;
    signed char small_int : 6;
    /* Force padding and unusual layout */
    unsigned char : 0; /* Zero-width bitfield forces alignment */
    short larger_short;
};

/* For DW_AT_lower_bound simulation */
template<int LowerBound, int UpperBound>
struct BoundedArray {
    static constexpr int lower = LowerBound;
    static constexpr int upper = UpperBound;
    static constexpr int size = UpperBound - LowerBound + 1;
    int data[size];
    
    int& operator[](int index) {
        return data[index - LowerBound];
    }
};

/* Complex location variable */
register int complex_location asm("ebx"); /* Hint for register storage */

#endif /* __cplusplus */

#endif /* DWARF_TEST_H */
