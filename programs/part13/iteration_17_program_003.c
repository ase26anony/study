/* Test program to trigger various DWARF attribute generation in GCC */
/* Compile with: gcc -g -dA -O0 -fopenmp -std=c17 test_dwarf_attrs.c */

#include <stddef.h>
#include <stdint.h>

/* For DW_AT_string_length, DW_AT_string_length_bit_size, DW_AT_string_length_byte_size */
/* Fixed-length string type that might trigger string length attributes */
typedef char fixed_string32[32];
typedef struct {
    char data[64];
    size_t length;  /* Explicit length field */
} string_with_len;

/* For DW_AT_picture_string - COBOL/Fortran PICTURE type simulation */
/* Use GCC's attribute if available */
#ifdef __GNUC__
struct picture_data {
    char value[15];
} __attribute__((packed));
#else
struct picture_data {
    char value[15];
};
#endif

/* For DW_AT_threads_scaled - OpenMP thread-related variables */
#ifdef _OPENMP
#include <omp.h>
int omp_global_var;
#pragma omp threadprivate(omp_global_var)

/* Thread-local storage */
__thread int thread_local_var;
#endif

/* For DW_AT_explicit - C++ only, handled in separate C++ file */
/* For DW_AT_is_optional - C++ only, handled in separate C++ file */

/* For DW_AT_mutable - C++ only, handled in separate C++ file */

/* For DW_AT_ordering - Array ordering (Fortran column-major) */
#ifdef __GNUC__
int column_major_array[5][5] __attribute__((aligned(16)));
#else
int column_major_array[5][5];
#endif

/* For DW_AT_segment - Segment-specific pointers */
/* Use compiler-specific segment attributes if available */
#ifdef __i386__
#ifdef __GNUC__
int * __attribute__((segment("fs"))) fs_seg_ptr;
int * __attribute__((segment("gs"))) gs_seg_ptr;
#endif
#endif

/* For DW_AT_prototyped - Function with prototype */
int prototyped_function(int a, char b, double c);
int (*func_ptr)(int, char, double) = &prototyped_function;

/* For DW_AT_small - Packed small types */
struct small_packed {
    unsigned int flag:1;
    unsigned int value:7;
} __attribute__((packed));

/* For DW_AT_location - Variables with specific locations */
register int reg_var asm("ebx");  /* May not work on all targets */

/* For DW_AT_lower_bound - Array with non-zero lower bound */
/* Simulate using pointer arithmetic */
typedef struct {
    int *data;
    int lower_bound;
    int upper_bound;
} bounded_array;

/* Complex type compositions to stress DIE generation */
typedef int (*complex_func_ptr)(int (*)(char), double[3][3]);

/* Storage class variations */
static int static_var = 42;
const int const_var = 100;
volatile int volatile_var;
restrict int *restrict_ptr;

/* Function implementations */
int prototyped_function(int a, char b, double c) {
    return a + (int)b + (int)c;
}

/* Main function using various constructs */
int main(void) {
    /* String length attributes test */
    fixed_string32 str1 = "Hello, World!";
    string_with_len str2 = {"Test", 4};
    
    /* Picture string simulation */
    struct picture_data pic = {"12345678901234"};
    
    /* Array ordering test */
    for (int i = 0; i < 5; i++) {
        for (int j = 0; j < 5; j++) {
            column_major_array[i][j] = i * 5 + j;
        }
    }
    
    /* Small packed struct */
    struct small_packed small = {1, 127};
    
    /* Bounded array simulation */
    int array_data[10];
    bounded_array barr = {array_data + 2, 2, 11};
    
    /* Use OpenMP if available */
    #ifdef _OPENMP
    #pragma omp parallel
    {
        omp_global_var = omp_get_thread_num();
        thread_local_var = omp_get_thread_num() * 10;
    }
    #endif
    
    /* Use function pointer */
    int result = func_ptr(42, 'A', 3.14);
    
    /* Use various storage classes */
    static_var++;
    volatile_var = static_var;
    int local_array[5] = {1, 2, 3, 4, 5};
    restrict_ptr = local_array;
    
    return result + static_var + small.value;
}

/* Additional global variables for more coverage */
extern int external_var;

/* Complex nested type */
typedef struct {
    struct {
        int x;
        int y;
    } point;
    union {
        int i;
        float f;
    } value;
} nested_type;

nested_type global_nested = {{1, 2}, {.i = 42}};

/* Variable with asm label */
int asm_var __asm__("custom_asm_name") = 99;

/* Weak symbol */
int weak_var __attribute__((weak)) = 0;

/* Aligned variable */
int aligned_var __attribute__((aligned(64)));
