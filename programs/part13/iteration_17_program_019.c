/*
 * Test program to trigger generation of specific DWARF attributes
 * Each section targets specific DWARF attributes from dwarf2out.cc
 */

#include <stddef.h>
#include <stdint.h>

/* For DW_AT_string_length, DW_AT_string_length_bit_size, DW_AT_string_length_byte_size */
/* Fixed-length string type (simulating Fortran CHARACTER) */
typedef char fixed_string[32];
fixed_string global_string = "test_string";

/* String with explicit length field */
struct string_with_len {
    int length;
    char data[64];
} string_var = {10, "test_data"};

/* For DW_AT_picture_string - COBOL/Fortran PICTURE data */
/* Use GCC attribute if available */
#ifdef __GNUC__
struct picture_data {
    char value[20];
} __attribute__((picture("9(5)V9(2)")));
#else
struct picture_data {
    char value[20];
};
#endif

/* For DW_AT_threads_scaled - OpenMP thread-related variables */
#ifdef _OPENMP
int omp_global_var = 42;
#pragma omp threadprivate(omp_global_var)

/* Thread-local storage */
__thread int thread_local_var = 100;
#endif

/* For DW_AT_explicit - C++ explicit constructor (in C we simulate with attributes) */
/* We'll handle C++ separately */

/* For DW_AT_is_optional - optional parameters/variables */
/* Simulated with pointer that can be NULL */
int *optional_ptr = NULL;

/* For DW_AT_mutable - C++ mutable (simulated in C with volatile) */
volatile int mutable_like = 0;

/* For DW_AT_ordering - array ordering (column-major) */
#ifdef __GNUC__
int column_major_array[5][5] __attribute__((column_major));
#else
int column_major_array[5][5];
#endif

/* For DW_AT_segment - segment-specific pointers */
#ifdef __i386__
/* x86 segment registers */
int __seg_fs *fs_seg_ptr;
int __seg_gs *gs_seg_ptr;
#endif

/* For DW_AT_prototyped - functions with prototypes */
int prototyped_function(int a, char b) {
    return a + b;
}

int (*func_ptr)(int, char) = &prototyped_function;

/* For DW_AT_small - packed/small types */
struct small_packed {
    unsigned int flag:1;
    unsigned int tiny:3;
} __attribute__((packed));

struct small_packed small_var = {1, 3};

/* Complex type compositions to stress DIE generation */
static int static_var = 42;
const int const_var = 100;
volatile int volatile_var = 200;
register int register_var asm("ebx");

/* Pointer to array of function pointers */
typedef int (*func_type)(int, int);
func_type func_array[5];

/* Nested structures */
struct outer {
    struct inner {
        int a;
        double b;
    } inner_struct;
    union {
        int x;
        float y;
    } inner_union;
};

/* Variable with multiple qualifiers */
const volatile int * restrict volatile complex_ptr;

/* Function with complex return type */
struct complex_return {
    int data[10];
    struct {
        int nested;
    } inner;
} get_complex(void) {
    struct complex_return cr = {{0}};
    return cr;
}

/* Main function using all variables */
int main(int argc, char *argv[]) {
    /* Use string variables */
    fixed_string local_string = "local";
    
    /* Use picture data */
    struct picture_data pic = {"1234567"};
    
    /* Use thread variables */
    #ifdef _OPENMP
    omp_global_var = argc;
    thread_local_var = argc * 2;
    #endif
    
    /* Use optional pointer */
    optional_ptr = &argc;
    
    /* Use mutable-like variable */
    mutable_like = argc;
    
    /* Use column-major array */
    for (int i = 0; i < 5; i++) {
        for (int j = 0; j < 5; j++) {
            column_major_array[i][j] = i * j;
        }
    }
    
    /* Use small packed struct */
    small_var.flag = argc > 0;
    small_var.tiny = argc & 0x7;
    
    /* Use function pointer */
    int result = func_ptr(argc, 'A');
    
    /* Use complex pointer */
    int target = 42;
    complex_ptr = &target;
    
    /* Use nested structure */
    struct outer o = {
        .inner_struct = {1, 2.0},
        .inner_union = {.x = 3}
    };
    
    /* Call complex function */
    struct complex_return cr = get_complex();
    
    return result + o.inner_struct.a + cr.inner.nested;
}
