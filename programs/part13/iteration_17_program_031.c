/*
 * Test program to trigger specific DWARF attribute generation in GCC
 * Each section targets specific DW_AT_* attributes from dwarf2out.cc
 */

#include <stddef.h>

/* For DW_AT_string_length, DW_AT_string_length_bit_size, DW_AT_string_length_byte_size */
/* Fixed-length string type - may trigger string length attributes */
typedef char fixed_string[32];
fixed_string global_string = "test_string";

/* For DW_AT_picture_string - COBOL/Fortran PICTURE data */
/* Use GCC attribute if available */
#ifdef __GNUC__
struct picture_data {
    char data[20];
} __attribute__((picture("999V99")));
#else
struct picture_data {
    char data[20];
};
#endif

/* For DW_AT_threads_scaled - OpenMP thread-related variables */
#ifdef _OPENMP
int omp_thread_var = 0;
#pragma omp threadprivate(omp_thread_var)

int omp_shared_var __attribute__((omp declare target));
#endif

/* For DW_AT_explicit - C++ explicit constructor (in C++ section) */
/* For DW_AT_is_optional - std::optional (in C++ section) */
/* For DW_AT_mutable - mutable member (in C++ section) */

/* For DW_AT_ordering - array ordering (Fortran column-major) */
#ifdef __GNUC__
int column_major_array[5][5] __attribute__((column_major));
#else
int column_major_array[5][5];
#endif

/* For DW_AT_segment - segment-specific pointers */
#ifdef __i386__
int __seg_fs *fs_seg_ptr;
int __seg_gs *gs_seg_ptr;
#endif

/* For DW_AT_prototyped - function with prototype */
int prototyped_function(int a, char b) {
    return a + b;
}

/* Function pointer with prototype */
int (*proto_func_ptr)(int, char) = &prototyped_function;

/* For DW_AT_small - packed struct with bit-field */
struct small_struct {
    unsigned int flag:1;
    unsigned int tiny:3;
} __attribute__((packed));

/* For DW_AT_location - variables with specific locations */
register int reg_var asm("ebx");  /* May not work on all targets */

/* For DW_AT_lower_bound - array with specified bounds */
#ifdef __GNUC__
int bounded_array[10] __attribute__((aligned(16)));
#else
int bounded_array[10];
#endif

/* Complex type compositions to stress DIE generation */
static const volatile int file_scope_cv = 42;
thread_local int tls_var = 100;
restrict int *restrict_ptr;

/* Nested structures */
struct outer {
    struct inner {
        int a;
        double b;
    } nested;
    int outer_member;
};

/* Union with bitfields */
union bit_union {
    struct {
        unsigned int a:4;
        unsigned int b:4;
        unsigned int c:8;
    } bits;
    unsigned short value;
};

/* Variable with __attribute__((aligned)) */
int aligned_var __attribute__((aligned(64)));

/* Array of function pointers */
int (*func_array[5])(void);

/* Main function with local variables */
int main(int argc, char *argv[]) {
    /* Local fixed string */
    fixed_string local_str = "local";
    
    /* Small struct */
    struct small_struct small = {1, 3};
    
    /* Picture data */
    struct picture_data pic = {{0}};
    
    /* Use column major array */
    for (int i = 0; i < 5; i++) {
        for (int j = 0; j < 5; j++) {
            column_major_array[i][j] = i * 5 + j;
        }
    }
    
    /* Use prototyped function */
    int result = prototyped_function(argc, 'A');
    
    /* Use bounded array */
    for (int i = 0; i < 10; i++) {
        bounded_array[i] = i * 2;
    }
    
    /* Complex local with restrict */
    int local_array[10];
    int *restrict local_restrict = local_array;
    for (int i = 0; i < 10; i++) {
        local_restrict[i] = i;
    }
    
    /* Nested struct */
    struct outer out = {{1, 2.0}, 3};
    
    /* Union */
    union bit_union bu = {{1, 2, 3}};
    
    /* Thread-local usage */
    tls_var += argc;
    
#ifdef _OPENMP
    /* OpenMP section for threads_scaled */
    #pragma omp parallel
    {
        omp_thread_var = omp_get_thread_num();
    }
#endif
    
    return result + small.flag + out.nested.a + bu.bits.a;
}

/* Additional function with inline assembly for location hints */
void asm_func(void) {
    int local_asm;
    asm volatile ("movl %%ebx, %0" : "=r"(local_asm));
}

/* Variable with section attribute */
int __attribute__((section(".mysection"))) section_var = 99;

/* Weak symbol */
int __attribute__((weak)) weak_var = 0;

/* Cleanup attribute */
void cleanup_func(int *p) {
    *p = 0;
}

void test_cleanup(void) {
    int __attribute__((cleanup(cleanup_func))) auto_clean = 42;
}
