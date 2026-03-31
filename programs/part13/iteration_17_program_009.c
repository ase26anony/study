/*
 * Test program to trigger generation of specific DWARF attributes
 * Each section targets specific DWARF attributes from dwarf2out.cc
 */

#include <stddef.h>
#include <stdint.h>

/* For DW_AT_string_length, DW_AT_string_length_bit_size, DW_AT_string_length_byte_size */
/* Fixed-length string type - may trigger string length attributes */
typedef char fixed_string_t[32];
fixed_string_t global_string = "Test string for length attributes";

/* For DW_AT_picture_string - COBOL/Fortran PICTURE data */
/* Use GCC's picture attribute if available */
#ifdef __GNUC__
struct picture_data {
    char data[20];
} __attribute__((picture("9(5)V9(2)")));
#else
/* Fallback - packed struct that might be interpreted as picture data */
#pragma pack(push, 1)
struct picture_data {
    char digits[7];
    char decimal;
    char fraction[2];
};
#pragma pack(pop)
#endif

/* For DW_AT_threads_scaled - OpenMP thread-related variables */
#ifdef _OPENMP
int omp_thread_var = 0;
#pragma omp threadprivate(omp_thread_var)

/* Thread-local storage */
__thread int thread_local_var = 42;
#endif

/* For DW_AT_segment - segment-specific pointers */
/* Use segment attributes for x86 memory models */
#ifdef __x86_64__
/* FS/GS segment pointers */
int * __seg_fs fs_seg_ptr = 0;
int * __seg_gs gs_seg_ptr = 0;
#endif

/* For DW_AT_ordering - array ordering (Fortran column-major) */
#ifdef __GNUC__
int column_major_array[5][5] __attribute__((column_major));
#else
int column_major_array[5][5];
#endif

/* For DW_AT_small - packed structures with bitfields */
struct small_packed_struct {
    unsigned int flag:1;
    unsigned int value:7;
} __attribute__((packed));

/* For DW_AT_prototyped - function with prototype */
int prototyped_function(int a, char b, double c);
int (*function_pointer)(int, char, double) = &prototyped_function;

/* Complex type compositions to stress DIE generation */
typedef struct {
    fixed_string_t name;
    int id;
    volatile int status;
} complex_type_t;

/* Mix of storage classes and qualifiers */
static const int static_const_int = 100;
volatile int volatile_global = 200;
register int register_var asm("r12") = 300;

/* Restrict qualified pointer */
void use_restrict(int *restrict ptr1, int *restrict ptr2) {
    *ptr1 += *ptr2;
}

/* Inline function with debug info */
static inline int inline_debug_func(int x) __attribute__((always_inline));
static inline int inline_debug_func(int x) {
    return x * 2;
}

/* Main test structures */
struct test_container {
    /* For DW_AT_mutable (C++ only, handled in separate C++ file) */
    /* mutable int mutable_member; */  /* C++ only */
    
    const fixed_string_t str_member;
    struct picture_data pic_member;
    struct small_packed_struct small_member;
    int (*func_ptr_member)(int, char, double);
};

/* Function implementations */
int prototyped_function(int a, char b, double c) {
    return a + (int)b + (int)c;
}

/* Array with complex indexing */
int multi_dim_array[3][4][5];

/* Union with bitfields for size optimization */
union compact_union {
    struct {
        unsigned int a:4;
        unsigned int b:4;
        unsigned int c:8;
        unsigned int d:16;
    } bits;
    uint32_t value;
};

/* Thread-related test function */
#ifdef _OPENMP
void test_omp_threads(void) {
    #pragma omp parallel
    {
        omp_thread_var = omp_get_thread_num();
        thread_local_var = omp_get_thread_num() * 100;
    }
}
#endif

/* Main function using all test constructs */
int main(void) {
    /* Local instances of test types */
    fixed_string_t local_string = "Local test string";
    struct picture_data local_picture;
    struct small_packed_struct local_small = {1, 127};
    struct test_container container = {
        .str_member = "Container string",
        .func_ptr_member = &prototyped_function
    };
    
    /* Use restrict pointers */
    int a = 10, b = 20;
    use_restrict(&a, &b);
    
    /* Use inline function */
    int doubled = inline_debug_func(a);
    
    /* Access column-major array */
    column_major_array[0][0] = 1;
    
    /* Use complex array */
    multi_dim_array[0][0][0] = 42;
    
    /* Use union */
    union compact_union cu;
    cu.bits.a = 0xF;
    cu.bits.b = 0xA;
    
    /* Test thread-related code */
    #ifdef _OPENMP
    test_omp_threads();
    #endif
    
    /* Use function pointer */
    int result = container.func_ptr_member(1, 'A', 3.14);
    
    /* Use segment pointers if available */
    #ifdef __x86_64__
    /* These would need proper initialization in real code */
    (void)fs_seg_ptr;
    (void)gs_seg_ptr;
    #endif
    
    return result + doubled + cu.value;
}
