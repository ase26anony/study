/*
 * Test program to trigger generation of specific DWARF attributes
 * in GCC's dwarf2out.cc (lines 7602-7643)
 */

#include <stddef.h>
#include <string.h>

/* For DW_AT_string_length, DW_AT_string_length_bit_size, DW_AT_string_length_byte_size */
/* Fixed-length string type */
typedef char fixed_string[32];
fixed_string global_string = "test_string";

/* String with explicit length (Fortran-style) */
struct fortran_string {
    int length;
    char data[1];
};

/* For DW_AT_picture_string - COBOL-style picture data */
/* Use GCC's attribute if available */
#ifdef __GNUC__
struct cobol_picture {
    char data[10];
} __attribute__((packed));
#else
struct cobol_picture {
    char data[10];
};
#endif

/* For DW_AT_threads_scaled - OpenMP thread-related */
#ifdef _OPENMP
#include <omp.h>
int omp_thread_var = 0;
#pragma omp threadprivate(omp_thread_var)
#endif

/* For DW_AT_explicit - C++ explicit constructor (in C++ section) */

/* For DW_AT_is_optional - C++17 optional (in C++ section) */

/* For DW_AT_mutable - C++ mutable member (in C++ section) */

/* For DW_AT_ordering - Column-major array ordering */
#ifdef __GNUC__
int column_major_array[5][5] __attribute__((column_major));
#else
int column_major_array[5][5];
#endif

/* For DW_AT_segment - Segment-specific pointers */
#ifdef __i386__
/* x86 segment registers */
int __seg_fs *fs_seg_ptr;
int __seg_gs *gs_seg_ptr;
#endif

/* For DW_AT_prototyped - Function with prototype */
int prototyped_function(int a, char b) {
    return a + b;
}

/* Function pointer with prototype */
int (*proto_func_ptr)(int, char) = &prototyped_function;

/* For DW_AT_small - Packed structure with bit-field */
struct small_packed {
    unsigned int flag:1;
    unsigned int value:7;
} __attribute__((packed));

/* Complex type compositions to stress DIE generation */
static const volatile int static_cv_int = 42;
register int reg_var asm("ebx");  /* May not work on all targets */

/* Thread-local storage */
_Thread_local int thread_local_var = 100;

/* Restrict-qualified pointer */
void use_restrict(int *restrict ptr1, int *restrict ptr2) {
    *ptr1 = *ptr2 + 1;
}

/* Array of function pointers */
typedef int (*func_ptr_t)(int);
func_ptr_t func_array[3];

/* Nested structures */
struct outer {
    struct inner {
        int a;
        double b;
    } nested;
    union {
        int x;
        float y;
    } data;
};

/* Variable with multiple storage classes and qualifiers */
static volatile const int complex_var = 0xDEADBEEF;

/* Inline function with debug info */
static inline int inline_func(int x) __attribute__((always_inline));
static inline int inline_func(int x) {
    return x * 2;
}

/* Main function with local variables in different scopes */
int main(int argc, char *argv[]) {
    /* Local fixed string */
    fixed_string local_str = "local_test";
    
    /* Packed structure */
    struct small_packed packed_var = {1, 42};
    
    /* Array with different ordering hint */
    int local_array[3][3];
    
    /* Use segment pointer if available */
#ifdef __i386__
    int local_fs_var = 0;
    fs_seg_ptr = &local_fs_var;
#endif
    
    /* Use OpenMP if available */
#ifdef _OPENMP
    #pragma omp parallel
    {
        omp_thread_var = omp_get_thread_num();
    }
#endif
    
    /* Use restrict */
    int a = 1, b = 2;
    use_restrict(&a, &b);
    
    /* Call prototyped function */
    int result = prototyped_function(argc, argv[0][0]);
    
    /* Use inline function */
    result = inline_func(result);
    
    /* Use thread-local */
    thread_local_var = result;
    
    return result;
}

/* Additional global to ensure DIE generation */
extern int external_var;
int *pointer_to_array[10];
const char *const_string = "constant_string";

/* Variable with asm label */
int asm_label_var asm("special_var") = 99;

/* Weak symbol */
int weak_var __attribute__((weak)) = 0;

/* Aligned variable */
int aligned_var __attribute__((aligned(64)));

/* Section-specific variable */
int section_var __attribute__((section(".mysection"))) = 123;

/* Cleanup attribute */
void cleanup_func(int *p) {
    *p = 0;
}

int cleanup_var __attribute__((cleanup(cleanup_func))) = 456;

/* Transparent union */
typedef union transparent_union {
    int *int_ptr;
    void *void_ptr;
} transparent_union_t __attribute__((transparent_union));

/* Mode attribute */
typedef int int32_alt __attribute__((mode(SI)));

/* Vector type */
typedef int v4si __attribute__((vector_size(16)));
v4si vector_var = {1, 2, 3, 4};
