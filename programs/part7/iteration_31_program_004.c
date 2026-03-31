// test_dwarf_attributes.h
#ifndef TEST_DWARF_ATTRIBUTES_H
#define TEST_DWARF_ATTRIBUTES_H

#include <stdarg.h>
#include <stdnoreturn.h>

// For DW_AT_segment
#ifdef __GNUC__
#define SECTION_ATTR __attribute__((section(".custom_section")))
#else
#define SECTION_ATTR
#endif

// For thread-local storage (DW_AT_threads_scaled)
_Thread_local int thread_local_var;

// Structure with various attributes
struct ComplexType {
    // DW_AT_mutable
    mutable int mutable_member;
    
    // Bit-field for string length attributes
    struct {
        unsigned int length_bits : 8;  // DW_AT_string_length_bit_size
        unsigned int length_bytes : 8; // DW_AT_string_length_byte_size
    } string_info;
    
    // Volatile for DW_AT_location variations
    volatile int volatile_member;
    
    // Const member
    const int const_member;
    
    // Array with explicit bounds
    int bounded_array[10];
};

// Function prototypes for DW_AT_prototyped
void prototyped_function(void);
int variadic_function(int count, ...);
noreturn void noreturn_function(void);

// Explicit template specialization for DW_AT_explicit (C++)
#ifdef __cplusplus
template<typename T>
class ExplicitTemplate {
public:
    explicit ExplicitTemplate(T value);  // explicit constructor
};

// Explicit specialization
template<>
class ExplicitTemplate<int> {
public:
    explicit ExplicitTemplate(int value);
};
#endif

// Global with segment attribute
SECTION_ATTR int segment_var = 42;

// Optional parameter simulation (DW_AT_is_optional)
struct OptionalParam {
    int is_present;
    union {
        int value;
        void* ptr;
    };
};

#endif // TEST_DWARF_ATTRIBUTES_H
