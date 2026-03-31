/* Target DWARF attributes through various language constructs */
#include "debug_types.h"
#include <stdio.h>
#include <stdarg.h>

/* For DW_AT_threads_scaled - thread-local storage */
_Thread_local int thread_local_var = 42;

/* For DW_AT_segment - segment attribute (GCC extension) */
#ifdef __GNUC__
int segment_var __attribute__((section(".my_section"))) = 100;
#endif

/* For DW_AT_prototyped - function prototypes */
void prototyped_func(int x, int y);
void variadic_func(const char* fmt, ...);
void void_func(void) __attribute__((noreturn));

/* For DW_AT_mutable - C++ only */
#ifdef __cplusplus
class MutableTest {
    mutable int mutable_member;
public:
    MutableTest() : mutable_member(0) {}
    void modify() const { mutable_member = 42; }  // Can modify mutable in const
};
#endif

/* For DW_AT_location - complex location descriptions */
volatile int volatile_var;
const int const_array[] = {1, 2, 3, 4, 5};

/* For DW_AT_lower_bound - array with non-zero lower bound */
struct array_with_bounds {
    int arr[10];
    int lower_bound;
};

/* For DW_AT_string_length_bit_size - bit-field string length */
struct string_with_bit_length {
    char data[100];
    unsigned int length : 7;  // 7-bit length field
};

/* For DW_AT_string_length_byte_size - byte-sized string length */
struct string_with_byte_length {
    char data[100];
    unsigned char length;  // byte length field
};

/* Dummy functions to ensure symbols are referenced */
void use_types(void) {
    /* Reference all types to prevent optimization */
    struct array_with_bounds bounds = {{0}, 1};
    struct string_with_bit_length bit_str = {"test", 4};
    struct string_with_byte_length byte_str = {"hello", 5};
    
    (void)bounds;
    (void)bit_str;
    (void)byte_str;
}

/* Variadic function for DW_AT_prototyped */
void variadic_func(const char* fmt, ...) {
    va_list args;
    va_start(args, fmt);
    vprintf(fmt, args);
    va_end(args);
}

int main(void) {
    int checksum = 0;
    
    /* Reference thread-local variable */
    checksum += thread_local_var;
    
    /* Reference segment variable */
    #ifdef __GNUC__
    checksum += segment_var;
    #endif
    
    /* Reference volatile variable */
    checksum += volatile_var;
    
    /* Reference const array */
    checksum += const_array[0];
    
    /* Use mutable class in C++ */
    #ifdef __cplusplus
    MutableTest mt;
    mt.modify();
    checksum += sizeof(mt);
    #endif
    
    /* Call variadic function */
    variadic_func("Test: %d\n", checksum);
    
    /* Use types from header */
    use_types();
    use_shared_types();
    
    /* Final output depends on addresses/sizes */
    printf("Result: %d\n", 
           checksum + 
           (int)((long)&thread_local_var % 100) +
           (int)(sizeof(struct string_with_bit_length)));
    
    return 0;
}
