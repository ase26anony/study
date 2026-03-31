/* test_dwarf_attrs.c - Comprehensive test for DWARF attribute generation */
/* Compile with: gcc -g -dA -O0 -fopenmp -std=c11 test_dwarf_attrs.c */

#include <stddef.h>
#include <stdint.h>

/* For DW_AT_string_length, DW_AT_string_length_bit_size, DW_AT_string_length_byte_size */
/* Fixed-length string type that might trigger string length attributes */
typedef char fixed_string_32[32];
typedef struct {
    char data[64];
    size_t length;  /* Explicit length field */
} string_with_len;

/* For DW_AT_picture_string - COBOL/Fortran PICTURE data */
/* Use GCC's attribute if available */
#ifdef __GNUC__
typedef char picture_string __attribute__((picture("9(5)V9(2)")));
#else
typedef char picture_string[10];
#endif

/* For DW_AT_threads_scaled - OpenMP thread-related variables */
#ifdef _OPENMP
#include <omp.h>
int omp_thread_var;
#pragma omp threadprivate(omp_thread_var)

/* Thread-local storage */
__thread int tls_var = 0;

/* OpenMP declare target */
#pragma omp declare target
int target_var = 42;
#pragma omp end declare target
#endif

/* For DW_AT_explicit - C++ only, handled in separate C++ file */
/* For DW_AT_is_optional - C++ only, handled in separate C++ file */

/* For DW_AT_mutable - C++ only, handled in separate C++ file */

/* For DW_AT_ordering - Array ordering (Fortran column-major) */
#ifdef __GNUC__
int column_major_array[5][5] __attribute__((column_major));
#else
int column_major_array[5][5];
#endif

/* For DW_AT_segment - Segment-specific pointers */
#if defined(__i386__) || defined(__x86_64__)
/* x86 segment registers */
int __seg_fs *fs_seg_ptr;
int __seg_gs *gs_seg_ptr;

/* Far pointers for 16-bit compatibility */
#ifdef __MSDOS__
int __far far_ptr;
#endif
#endif

/* For DW_AT_prototyped - Function prototypes */
int prototyped_function(int a, char b, double c);
int (*prototyped_func_ptr)(int, char, double) = &prototyped_function;

/* For DW_AT_small - Packed structures and bit-fields */
struct small_packed {
    unsigned int flag1:1;
    unsigned int flag2:1;
    unsigned int value:6;
} __attribute__((packed));

/* For DW_AT_location - Variables with complex locations */
register int reg_var asm("ebx");  /* May not work on all targets */

/* For DW_AT_lower_bound - Array with explicit bounds */
typedef int bounded_array[10];
struct with_bounds {
    int array[5];
    int lower_bound;
    int upper_bound;
};

/* For DW_AT_is_optional - Simulate with union for C */
union optional_int {
    int value;
    struct { unsigned char is_present; } state;
};

/* Complex type compositions to stress DIE generation */
typedef int (*complex_func_ptr)(int (*)(char), double[3]);
typedef struct nested {
    struct inner {
        int x;
        volatile char y;
    } inner_struct;
    const struct nested *next;
    restrict int *restrict_ptr;
} nested_struct;

/* Storage class variations */
static int static_var = 100;
extern int extern_var;
volatile int volatile_var = 200;
const int const_var = 300;
_Atomic int atomic_var = 400;

/* Function with mixed parameters */
int mixed_params(const char *str, volatile int *ptr, register int reg, 
                 int array[static 5], ...);

/* Variable in different scopes */
void function_scope_test(void) {
    auto int auto_var = 50;
    static int func_static = 60;
    const int func_const = 70;
    
    /* Array with different storage */
    int local_array[10] = {0};
    static int static_local_array[10] = {0};
    
    /* Pointer with restrict */
    int *restrict local_restrict_ptr = local_array;
    
    /* Use variables to avoid warnings */
    (void)auto_var;
    (void)func_static;
    (void)func_const;
    (void)local_array;
    (void)static_local_array;
    (void)local_restrict_ptr;
}

/* Actual function definitions */
int prototyped_function(int a, char b, double c) {
    return a + (int)b + (int)c;
}

int mixed_params(const char *str, volatile int *ptr, register int reg,
                 int array[static 5], ...) {
    return *ptr + reg + array[0];
}

/* Main function to use all variables */
int main(void) {
    /* String length attributes */
    fixed_string_32 fs = "Fixed string test";
    string_with_len swl = {.data = "Dynamic string", .length = 14};
    
    /* Picture string */
    picture_string ps = "123456789";
    
    /* Packed structure */
    struct small_packed sp = {.flag1 = 1, .flag2 = 0, .value = 42};
    
    /* Array ordering */
    for (int i = 0; i < 5; i++) {
        for (int j = 0; j < 5; j++) {
            column_major_array[i][j] = i * 5 + j;
        }
    }
    
#ifdef _OPENMP
    /* OpenMP section for threads_scaled */
    #pragma omp parallel
    {
        omp_thread_var = omp_get_thread_num();
        tls_var = omp_get_thread_num() * 10;
        
        #pragma omp target
        {
            target_var = omp_get_thread_num() * 100;
        }
    }
#endif
    
    /* Use segment pointers if available */
#if defined(__i386__) || defined(__x86_64__)
    /* These are just declarations - actual usage requires OS support */
    (void)fs_seg_ptr;
    (void)gs_seg_ptr;
#endif
    
    /* Call prototyped function */
    int result = prototyped_function(10, 'A', 3.14);
    
    /* Use function pointer */
    result += prototyped_func_ptr(20, 'B', 2.71);
    
    /* Test mixed params */
    volatile int vi = 100;
    int arr[5] = {1, 2, 3, 4, 5};
    result += mixed_params("test", &vi, 50, arr);
    
    /* Test optional */
    union optional_int opt = {.value = 999};
    
    /* Call scope test */
    function_scope_test();
    
    /* Use all global variables to avoid unused warnings */
    result += static_var + volatile_var + const_var + atomic_var;
    
    /* Use local variables */
    (void)fs;
    (void)swl;
    (void)ps;
    (void)sp;
    (void)opt;
    
    return result > 0 ? 0 : 1;
}
