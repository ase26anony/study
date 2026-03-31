/* Main driver to ensure all debug symbols are referenced */
#include <stdio.h>
#include <stdint.h>

/* Forward declarations from other modules */
extern void ada_test(void);
extern void fortran_test(void);
extern int cpp_test(void);

/* For DW_AT_segment - use segment attribute if supported */
#ifdef __GNUC__
#define SEGMENT(name) __attribute__((section(#name)))
#else
#define SEGMENT(name)
#endif

/* For DW_AT_threads_scaled - thread-local storage */
_Thread_local int thread_var SEGMENT(.tdata) = 42;

/* For DW_AT_location variations */
volatile int global_volatile = 100;
const int global_const = 200;

/* For DW_AT_lower_bound - array with explicit bounds */
int bounded_array[5] = {1, 2, 3, 4, 5};

/* For DW_AT_string_length_bit_size - bit-field structures */
struct string_info {
    unsigned int length : 12;      /* 12-bit length field */
    unsigned int encoding : 4;     /* 4-bit encoding */
    unsigned int : 0;              /* Force alignment */
    char data[];
};

/* For DW_AT_prototyped - various function prototypes */
void void_func(void);                     /* DW_AT_prototyped with void */
int int_func(int, float);                 /* DW_AT_prototyped with params */
void variadic_func(const char*, ...);     /* DW_AT_prototyped variadic */
_Noreturn void noreturn_func(void);       /* DW_AT_prototyped noreturn */

/* Function definitions */
void void_func(void) {
    /* Empty but referenced */
}

int int_func(int a, float b) {
    return a + (int)b;
}

void variadic_func(const char* fmt, ...) {
    /* Dummy implementation */
    (void)fmt;
}

_Noreturn void noreturn_func(void) {
    while(1);  /* Infinite loop for noreturn */
}

/* Main function that references everything */
int main(void) {
    int checksum = 0;
    
    /* Reference thread-local variable */
    checksum ^= thread_var;
    
    /* Reference volatile and const */
    checksum ^= global_volatile;
    checksum ^= global_const;
    
    /* Reference array with bounds */
    for(int i = 0; i < 5; i++) {
        checksum ^= bounded_array[i];
    }
    
    /* Reference bit-field structure */
    struct string_info info = {0};
    checksum ^= info.length;
    
    /* Call functions with prototypes */
    void_func();
    checksum ^= int_func(10, 3.14f);
    
    /* Call Ada and Fortran tests if available */
    #ifdef WITH_ADA
    ada_test();
    #endif
    
    #ifdef WITH_FORTRAN
    fortran_test();
    #endif
    
    /* Call C++ test */
    checksum ^= cpp_test();
    
    /* Print non-constant result */
    printf("Debug test checksum: %d\n", checksum);
    
    return checksum & 0xFF;
}
