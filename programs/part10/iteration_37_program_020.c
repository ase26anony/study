#ifndef DWARF_ATTRIBUTES_H
#define DWARF_ATTRIBUTES_H

#include <optional>
#include <cstddef>

#ifdef __cplusplus
extern "C" {
#endif

/* For DW_AT_prototyped - old K&R style prototype */
int old_style_function(); /* No parameter list in prototype */

/* For DW_AT_picture_string - simulate COBOL-like type */
typedef char PICTURE[20];
struct CobolLike {
    PICTURE picture_field;
};

/* For DW_AT_segment */
#ifdef __GNUC__
#define SECTION_ATTR __attribute__((section(".mysection")))
#else
#define SECTION_ATTR
#endif

extern int SECTION_ATTR segment_var;

/* For DW_AT_string_length structures */
struct StringWithLength {
    size_t length;
    char data[];
};

struct BoundedString {
    int length_bits;
    int length_bytes;
    char* str;
};

/* For DW_AT_threads_scaled */
#ifdef _OPENMP
extern int thread_scaled_array[100];
#endif

/* For DW_AT_lower_bound simulation */
struct ArrayWithLowerBound {
    int* data;
    int lower_bound;
    int upper_bound;
};

#ifdef __cplusplus
} /* extern "C" */

/* For DW_AT_explicit and DW_AT_mutable */
class ExplicitConstructor {
    int value;
public:
    explicit ExplicitConstructor(int v) : value(v) {}
    int get() const { return value; }
};

class WithMutable {
    mutable int counter;
    int value;
public:
    WithMutable(int v) : counter(0), value(v) {}
    void increment() const { ++counter; }  // Can modify mutable member
    int get_counter() const { return counter; }
};

/* For DW_AT_ordering */
enum OrderedEnum {
    FIRST = 10,
    SECOND = 5,
    THIRD = 20,
    FOURTH = 1
};

/* For DW_AT_small */
struct __attribute__((packed)) PackedStruct {
    unsigned char small_char : 4;
    unsigned char tiny_bool : 1;
    unsigned char another_small : 3;
    int regular_int;
};

/* Template to force emission in multiple contexts */
template<typename T>
class DebugTemplate {
    T value;
public:
    explicit DebugTemplate(T v) : value(v) {}
    T get() const { return value; }
    
    // Complex method that might generate interesting locations
    T process() {
        register T local = value;  // register storage class
        for (int i = 0; i < 10; ++i) {
            local += static_cast<T>(i);
        }
        return local;
    }
};

/* Inline function with complex variable locations */
inline int complex_location_function(int x) {
    // Variable with potentially complex location
    volatile int a = x;
    volatile int b = x * 2;
    register int c __asm__("eax") = a + b;  // Suggest specific register
    
    // Split variable across conceptual registers
    int result;
    {
        register int temp = c;
        result = temp * 3;
    }
    
    return result;
}

#endif /* __cplusplus */

#endif /* DWARF_ATTRIBUTES_H */
