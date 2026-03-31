/* test_dwarf_attrs.c - Comprehensive test for triggering specific DWARF attributes */
/* Compile with: gcc -g -dA -O0 -fopenmp -std=c++17 test_dwarf_attrs.c -o test_dwarf_attrs */

#include <stddef.h>
#include <stdint.h>

/* For DW_AT_string_length, DW_AT_string_length_bit_size, DW_AT_string_length_byte_size */
/* Fixed-length string type that might trigger string length attributes */
typedef char fixed_string_t[32];
fixed_string_t global_string = "Test string for DW_AT_string_length";

/* Struct with string member */
struct string_struct {
    fixed_string_t str;
    int length;
};

/* For DW_AT_picture_string - COBOL/Fortran PICTURE data simulation */
/* Using GCC's __attribute__ for picture strings if available */
#ifdef __GNUC__
struct picture_data {
    char data[20];
} __attribute__((packed));
#else
struct picture_data {
    char data[20];
};
#endif

/* For DW_AT_threads_scaled - OpenMP thread-related variables */
#ifdef _OPENMP
#include <omp.h>
int omp_global_var = 0;
#pragma omp threadprivate(omp_global_var)
#endif

/* Thread-local storage */
_Thread_local int thread_local_var = 42;

/* For DW_AT_segment - Segment-specific pointers */
/* Using segment attributes for x86 memory models */
#ifdef __x86_64__
int * __attribute__((address_space(256))) fs_ptr;  /* FS segment simulation */
int * __attribute__((address_space(257))) gs_ptr;  /* GS segment simulation */
#endif

/* For DW_AT_prototyped - Function with prototype */
int prototyped_function(int a, char b, double c);
int (*func_ptr)(int, char, double) = &prototyped_function;

/* For DW_AT_small - Packed struct with bit-fields */
struct small_struct {
    unsigned int flag:1;
    unsigned int value:7;
} __attribute__((packed));

/* For DW_AT_ordering - Array ordering (column-major simulation) */
#ifdef __GNUC__
int column_major_array[5][5] __attribute__((aligned(16)));
#else
int column_major_array[5][5];
#endif

/* For DW_AT_lower_bound - Array with explicit bounds */
struct bounded_array {
    int data[10];
    int lower_bound;
    int upper_bound;
};

/* For DW_AT_location - Variables with complex locations */
register int reg_var asm("r12");  /* Register variable */

/* Complex type compositions to stress DIE generation */
typedef int (*complex_func_t)(int, ...);
typedef struct string_struct *(*func_returning_struct_ptr_t)(void);

/* Storage class variations */
static int static_var = 100;
const int const_var = 200;
volatile int volatile_var = 300;
restrict int *restrict_ptr;

/* Function implementations */
int prototyped_function(int a, char b, double c) {
    return a + (int)b + (int)c;
}

/* Main function with various scopes */
int main(int argc, char *argv[]) {
    /* Local variables in different scopes */
    {
        /* Block scope variables */
        fixed_string_t local_string = "Local string";
        struct string_struct local_struct = { .str = "Struct string", .length = 13 };
        
        /* For DW_AT_mutable - C++ mutable simulation */
        /* In C, we simulate with volatile to force different handling */
        volatile int mutable_like = 999;
        
        /* Array with ordering */
        for (int i = 0; i < 5; i++) {
            for (int j = 0; j < 5; j++) {
                column_major_array[i][j] = i * 5 + j;
            }
        }
    }
    
    /* For DW_AT_is_optional - Optional parameter simulation */
    /* Using pointer with NULL as "optional" */
    int *optional_ptr = NULL;
    if (argc > 1) {
        optional_ptr = &argc;
    }
    
    /* For DW_AT_explicit - Explicit conversion simulation */
    /* In C, we use explicit casts */
    int explicit_val = (int)3.14159;
    
#ifdef _OPENMP
    /* OpenMP region for thread-scaled variables */
    #pragma omp parallel
    {
        omp_global_var = omp_get_thread_num();
    }
#endif
    
    /* Use all variables to prevent optimization */
    static_var += global_string[0];
    volatile_var += const_var;
    
    if (restrict_ptr) {
        *restrict_ptr = static_var;
    }
    
    /* Call prototyped function */
    int result = prototyped_function(10, 'A', 3.14);
    
    /* Use thread-local variable */
    thread_local_var += result;
    
    return thread_local_var;
}

/* Additional global variables with complex types */
complex_func_t global_func_ptr = NULL;
func_returning_struct_ptr_t struct_func_ptr = NULL;

/* Union with bit-fields for small representation */
union small_union {
    struct {
        unsigned int a:4;
        unsigned int b:4;
        unsigned int c:4;
        unsigned int d:4;
    } bits;
    unsigned short value;
} __attribute__((packed));

/* Enum with explicit values */
enum ordered_enum {
    ENUM_FIRST = 0,
    ENUM_SECOND = 1,
    ENUM_THIRD = 2
};

/* Typedef chain for complex DIE generation */
typedef int base_type;
typedef base_type *ptr_type;
typedef ptr_type array_of_ptrs_t[5];
typedef array_of_ptrs_t *ptr_to_array_of_ptrs_t;

/* Variable using complex typedef chain */
array_of_ptrs_t complex_var;

/* Function with variable arguments for prototyped attribute */
int varargs_function(int count, ...) {
    return count * 2;
}

/* Inline function */
static inline int inline_function(int x) {
    return x * x;
}

/* Weak symbol */
int weak_var __attribute__((weak)) = 123;

/* Aligned variable */
int aligned_var __attribute__((aligned(64)));

/* Section-specific variable */
int section_var __attribute__((section(".data.custom")));

/* Cleanup attribute */
void cleanup_handler(void *p) {
    *(int *)p = 0;
}

/* Function using cleanup */
void function_with_cleanup(void) {
    int x __attribute__((cleanup(cleanup_handler))) = 100;
    /* x will be cleaned up automatically */
}
