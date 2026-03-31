/* test_dwarf_attrs.c - Comprehensive test for triggering specific DWARF attributes */
/* Compile with: gcc -g -O0 -fopenmp -std=c11 test_dwarf_attrs.c -o test_dwarf_attrs */

#include <stddef.h>
#include <stdint.h>

/* For DW_AT_string_length, DW_AT_string_length_bit_size, DW_AT_string_length_byte_size */
/* Fixed-length string type that might trigger string length attributes */
typedef char fixed_string_t[32];
typedef struct {
    char data[64];
    size_t length;  /* Might trigger string length attributes in debug info */
} string_with_len_t;

/* For DW_AT_picture_string - COBOL/Fortran PICTURE data simulation */
/* Use GCC's __attribute__ for picture strings if available */
#ifdef __GNUC__
    typedef struct {
        char picture_data[20];
    } __attribute__((packed)) cobol_picture_t;
#else
    typedef struct {
        char picture_data[20];
    } cobol_picture_t;
#endif

/* For DW_AT_threads_scaled - OpenMP thread-related variables */
#pragma omp declare target
int target_var = 42;
#pragma omp end declare target

/* Thread-local storage that might trigger threads_scaled */
_Thread_local int thread_specific_var = 100;

/* For DW_AT_segment - Segment-specific pointers */
/* Use compiler-specific segment attributes */
#ifdef __x86_64__
    /* FS/GS segment registers for thread-local storage */
    #define __seg_fs __attribute__((address_space(257)))
    #define __seg_gs __attribute__((address_space(256)))
    int __seg_fs *fs_ptr = 0;
    int __seg_gs *gs_ptr = 0;
#elif defined(__i386__)
    /* Near/far pointers for x86 real mode */
    #define far __far
    #define near __near
    int far *far_ptr = 0;
    int near *near_ptr = 0;
#endif

/* For DW_AT_prototyped - Function with prototype */
int prototyped_function(int a, char b, double c);
int (*func_ptr)(int, char, double) = &prototyped_function;

/* For DW_AT_small - Packed structure with bit-fields */
struct small_packed_struct {
    unsigned int flag1 : 1;
    unsigned int flag2 : 1;
    unsigned int value : 6;
} __attribute__((packed));

/* For DW_AT_ordering - Column-major array ordering (Fortran style) */
#ifdef __GNUC__
    int column_major_array[5][5] __attribute__((column_major));
#else
    int column_major_array[5][5];
#endif

/* Complex type composition to stress DIE generation */
typedef struct {
    int id;
    fixed_string_t name;
    void (*callback)(void);
} complex_type_t;

/* Variables with different storage classes */
static int static_var = 123;
register int reg_var asm ("r12") = 456;  /* Hint to use register */
volatile int volatile_var = 789;
const int const_var = 999;
restrict int *restrict_ptr = NULL;

/* Function declarations with various attributes */
void __attribute__((noinline)) noinline_func(void) {
    static int local_static = 0;
    local_static++;
}

int __attribute__((always_inline)) inline_func(int x) {
    return x * 2;
}

/* Main test function */
int prototyped_function(int a, char b, double c) {
    /* Local variables with different types */
    fixed_string_t local_string = "Hello, DWARF!";
    cobol_picture_t local_picture;
    struct small_packed_struct local_small = {1, 0, 42};
    
    /* Array with different ordering hints */
    int row_major[3][3] = {{1,2,3},{4,5,6},{7,8,9}};
    
    /* OpenMP parallel region for thread scaling */
    #pragma omp parallel
    {
        int thread_id = 0;
        #ifdef _OPENMP
        thread_id = omp_get_thread_num();
        #endif
        thread_specific_var = thread_id;
    }
    
    /* Use segment pointers if available */
    #ifdef __x86_64__
        asm volatile("" : : "r"(fs_ptr), "r"(gs_ptr));
    #endif
    
    return a + (int)b + (int)c;
}

/* Additional test cases in different scopes */
namespace test_namespace {
    int namespace_var = 111;
    
    typedef struct {
        int x;
        int y;
    } point_t;
}

/* Block scope with register variable */
void test_block_scope(void) {
    {
        register int block_reg asm ("r13") = 222;
        volatile int block_volatile = 333;
        asm volatile("" : : "r"(block_reg));
    }
}

/* Union with bit-fields for small representation */
union bitfield_union {
    struct {
        unsigned int a : 4;
        unsigned int b : 4;
        unsigned int c : 4;
        unsigned int d : 4;
    } bits;
    unsigned short all;
} __attribute__((packed));

/* Main function */
int main(void) {
    /* Initialize and use all test variables */
    fixed_string_t my_string = "Test string";
    cobol_picture_t my_picture;
    struct small_packed_struct my_small = {0, 1, 31};
    
    /* Call functions */
    int result = prototyped_function(10, 'A', 3.14);
    noinline_func();
    
    /* Use OpenMP */
    #pragma omp parallel for
    for (int i = 0; i < 10; i++) {
        target_var += i;
    }
    
    /* Use complex type */
    complex_type_t complex_var = {
        .id = 1,
        .name = "Complex",
        .callback = NULL
    };
    
    /* Use ordering attribute array */
    for (int i = 0; i < 5; i++) {
        for (int j = 0; j < 5; j++) {
            column_major_array[i][j] = i * 5 + j;
        }
    }
    
    test_block_scope();
    
    return result + static_var + volatile_var + const_var;
}

/* Force generation of debug info for unused types */
typedef struct {
    int force_die_generation;
} unused_type_t;

/* Additional stress: pointer to array of function pointers */
typedef int (*func_array_t[10])(void);
func_array_t func_array;

/* Variable with alignment attribute */
int __attribute__((aligned(64))) aligned_var = 0;
