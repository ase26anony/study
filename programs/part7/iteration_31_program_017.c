/* Main driver with C++ features for explicit, mutable, etc. */
#include <stdio.h>
#include <stdarg.h>

/* External declarations from other modules */
extern void ada_test(void);
extern void fortran_test(void);

/* For DW_AT_segment */
#ifdef __GNUC__
#define SEGMENT(name) __attribute__((section(#name)))
#else
#define SEGMENT(name)
#endif

/* For DW_AT_threads_scaled */
_Thread_local int thread_local_var = 42;

/* For DW_AT_prototyped */
void prototyped_func(void) {
    printf("Fully prototyped\n");
}

void variadic_func(const char* fmt, ...) {
    va_list args;
    va_start(args, fmt);
    vprintf(fmt, args);
    va_end(args);
}

/* For DW_AT_location variations */
volatile int volatile_var = 100;
const int const_var = 200;

/* For DW_AT_lower_bound */
int array_with_bounds[5] = {1, 2, 3, 4, 5};

/* Structure with various attributes */
struct complex_struct {
    int normal;
    mutable int mutable_member;  /* For DW_AT_mutable (C++) */
    volatile int volatile_member;
    const int const_member;
    unsigned int bitfield : 4;   /* For bit-size attributes */
    unsigned int : 4;            /* Unnamed bitfield */
} SEGMENT(.data) global_struct;  /* Segment attribute */

/* Enum with explicit values */
enum ordering_enum {
    FIRST = 1,
    SECOND = 2,
    THIRD = 3
};

/* Function using thread-local storage */
int use_thread_local(void) {
    return thread_local_var++;
}

/* Prevent optimization */
__attribute__((noinline)) 
int compute_checksum(void* ptr) {
    return (int)((long)ptr & 0xFFFF);
}

int main(void) {
    int checksum = 0;
    
    /* Reference all variables to prevent optimization */
    checksum ^= compute_checksum(&volatile_var);
    checksum ^= compute_checksum(&const_var);
    checksum ^= compute_checksum(array_with_bounds);
    checksum ^= compute_checksum(&global_struct);
    
    /* Use functions */
    prototyped_func();
    variadic_func("Checksum: %d\n", checksum);
    
    /* Thread-local usage */
    checksum ^= use_thread_local();
    
    /* Call Ada and Fortran tests if available */
    #ifdef WITH_ADA
    ada_test();
    #endif
    
    #ifdef WITH_FORTRAN
    fortran_test();
    #endif
    
    printf("Final checksum: %d\n", checksum);
    return checksum & 1;  /* Non-constant return */
}
