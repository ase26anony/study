#ifndef DWARF_ATTRIBUTES_H
#define DWARF_ATTRIBUTES_H

#include <optional>
#include <cstddef>

#ifdef __cplusplus
extern "C" {
#endif

// For DW_AT_prototyped - old K&R style function declaration
int old_style_function();  // No parameter list - triggers prototyped attribute

// For DW_AT_picture_string - simulate COBOL-like picture type
typedef char PICTURE[20];
struct CobolLike {
    PICTURE picture_field;
};

#ifdef __cplusplus
}
#endif

// For DW_AT_explicit
class ExplicitConstructor {
    int value;
public:
    explicit ExplicitConstructor(int v) : value(v) {}
    int get() const { return value; }
};

// For DW_AT_mutable
class WithMutable {
    mutable int counter;
    int value;
public:
    WithMutable(int v) : counter(0), value(v) {}
    void increment() const { ++counter; }  // Can modify mutable member
    int get_counter() const { return counter; }
};

// For DW_AT_ordering
enum OrderedEnum {
    FIRST = 10,
    SECOND = 5,    // Not in declaration order
    THIRD = 20,
    FOURTH = 15    // Also out of order
};

// For DW_AT_small - packed struct with bitfields
struct __attribute__((packed)) SmallPacked {
    unsigned char tiny : 3;    // Very small bitfield
    unsigned char small : 5;
    unsigned char normal;
    unsigned int large;
};

// For DW_AT_string_length family
struct StringWithLength {
    size_t length;
    char data[];  // Flexible array member
};

// For DW_AT_lower_bound simulation
template<typename T, int LOWER_BOUND, int UPPER_BOUND>
struct BoundedArray {
    T data[UPPER_BOUND - LOWER_BOUND + 1];
    T& operator[](int index) { return data[index - LOWER_BOUND]; }
    const T& operator[](int index) const { return data[index - LOWER_BOUND]; }
};

// For complex DW_AT_location
volatile int global_seed = 42;

int get_complex_value() {
    register int local_reg __asm__("eax") = global_seed;
    // Force variable to be in register then memory
    int result;
    __asm__ volatile (
        "movl %1, %0\n\t"
        "addl $1, %0"
        : "=r"(result)
        : "r"(local_reg)
    );
    return result;
}

// For DW_AT_threads_scaled
#ifdef _OPENMP
extern int thread_scaled_array[];
#else
extern thread_local int thread_scaled_array[100];
#endif

#endif // DWARF_ATTRIBUTES_H
