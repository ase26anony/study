/* Test program to trigger specific DWARF attribute generation */
#include "debug_types.h"
#include <stdio.h>
#include <stdarg.h>

/* Global variables with various attributes */
__attribute__((section("MYSEGMENT"))) int segment_var = 42;  /* For DW_AT_segment */

/* Thread-local storage */
_Thread_local int thread_scaled = 100;  /* For DW_AT_threads_scaled */

/* Function prototypes */
int prototyped_func(int a, int b);  /* For DW_AT_prototyped */
void variadic_func(const char* fmt, ...);  /* Variadic function */
__attribute__((noreturn)) void no_return_func(void);

/* Structure with mutable member (C++) */
#ifdef __cplusplus
struct TestStruct {
    int normal;
    mutable int mutable_member;  /* For DW_AT_mutable */
    volatile int volatile_member;
    const int const_member;
    
    /* Array with explicit bounds */
    int bounded_array[10];
    
    /* Bit-field for string length attributes */
    struct {
        unsigned int length_bits : 8;
        unsigned int length_bytes : 16;
    } string_info;
};
#endif

/* Union with ordering */
union OrderedUnion {
    int i;
    float f;
    char bytes[4];
};

/* Enum with explicit values */
enum OrderedEnum {
    FIRST = 1,
    SECOND = 2,
    THIRD = 3
};

/* Function using variadic arguments */
void variadic_func(const char* fmt, ...) {
    va_list args;
    va_start(args, fmt);
    vprintf(fmt, args);
    va_end(args);
}

/* Prototyped function */
int prototyped_func(int a, int b) {
    return a + b;
}

/* No-return function */
__attribute__((noreturn)) void no_return_func(void) {
    while(1);  /* Infinite loop */
    __builtin_unreachable();
}

/* Main function that references everything */
int main(void) {
    int checksum = 0;
    
    /* Reference segment variable */
    checksum += segment_var;
    
    /* Reference thread-local variable */
    checksum += thread_scaled;
    
#ifdef __cplusplus
    /* C++ specific tests */
    TestStruct ts;
    ts.mutable_member = 5;
    ts.volatile_member = 10;
    
    /* Use array with bounds */
    for (int i = 0; i < 10; i++) {
        ts.bounded_array[i] = i;
        checksum += ts.bounded_array[i];
    }
    
    checksum += ts.mutable_member;
    checksum += ts.volatile_member;
    
    /* Set bit-field values */
    ts.string_info.length_bits = 32;
    ts.string_info.length_bytes = 4;
    checksum += ts.string_info.length_bits;
    checksum += ts.string_info.length_bytes;
#endif
    
    /* Use ordered types */
    union OrderedUnion ou;
    ou.i = 0x12345678;
    checksum += ou.bytes[0];
    
    enum OrderedEnum oe = SECOND;
    checksum += oe;
    
    /* Call prototyped functions */
    checksum += prototyped_func(1, 2);
    variadic_func("Test: %d\n", checksum);
    
    /* Reference external types */
    checksum += sizeof(ExternalStruct);
    checksum += sizeof(ExternalUnion);
    
    /* Prevent optimization */
    volatile int result = checksum;
    printf("Final checksum: %d\n", result);
    
    return result % 256;
}
