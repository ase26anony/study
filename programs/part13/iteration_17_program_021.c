/* test_dwarf_attrs.c - Comprehensive test for various DWARF attributes */
/* Compile with: gcc -g -dA -O0 -fopenmp -std=c++17 test_dwarf_attrs.c -o test_dwarf_attrs */

#include <stddef.h>
#include <stdint.h>

/* For DW_AT_string_length, DW_AT_string_length_bit_size, DW_AT_string_length_byte_size */
/* Fixed-length string array - may trigger string length attributes */
typedef char fixed_string[32];
fixed_string global_string = "test_string_for_dwarf";

/* For DW_AT_picture_string - COBOL/Fortran PICTURE data simulation */
/* Use GCC attribute if available */
#ifdef __GNUC__
struct picture_data {
    char data[20];
} __attribute__((packed));
#else
#pragma pack(push, 1)
struct picture_data {
    char data[20];
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

/* For DW_AT_explicit - C++ explicit constructor (in C we simulate with attributes) */
/* We'll handle C++ separately in a .cpp file */

/* For DW_AT_is_optional - Simulate optional parameters */
/* In C, use pointers that can be NULL */
struct optional_data {
    int *optional_ptr;  /* NULL indicates optional */
    int value;
};

/* For DW_AT_mutable - C++ mutable (simulated in C) */
struct mutable_sim {
    int regular;
    volatile int mutable_like;  /* volatile as mutable analog */
};

/* For DW_AT_ordering - Array ordering attributes */
#ifdef __GNUC__
int column_major_array[5][5] __attribute__((column_major));
#else
int column_major_array[5][5];
#endif

/* For DW_AT_segment - Segment-specific pointers */
/* Use compiler-specific segment attributes if available */
#ifdef __i386__
int __seg_fs *fs_segment_ptr;
int __seg_gs *gs_segment_ptr;
#elif defined(__x86_64__)
/* x86_64 uses FS/GS for thread-local storage */
int *fs_base_ptr __attribute__((address_space(257)));  /* FS segment */
int *gs_base_ptr __attribute__((address_space(256)));  /* GS segment */
#endif

/* For DW_AT_prototyped - Functions with prototypes */
int prototyped_function(int a, char b, double c);
int (*func_ptr)(int, char, double) = &prototyped_function;

/* For DW_AT_small - Packed structures with bit-fields */
struct small_packed {
    unsigned int flag1 : 1;
    unsigned int flag2 : 1;
    unsigned int flag3 : 1;
    unsigned int flag4 : 1;
    unsigned int : 4;  /* padding */
} __attribute__((packed));

/* Complex type compositions to stress DIE generation */
typedef struct {
    fixed_string name;
    int id;
    struct picture_data *pic;
} complex_type;

/* Function with mixed parameters */
int mixed_params(const char *str, volatile int *ptr, 
                 struct small_packed *sp, 
                 int (*callback)(int, int));

/* Variables with different storage classes */
static int static_var = 100;
register int register_var asm("ebx");  /* Hint to compiler */
const int const_var = 200;
volatile int volatile_var = 300;
restrict int *restrict_ptr;

/* Nested structures */
struct outer {
    int a;
    struct inner {
        int b;
        char c;
        struct innermost {
            short s;
        } deepest;
    } nested;
    double d;
};

/* Array of function pointers */
int (*func_array[5])(int, int);

/* Union with bit-fields */
union bit_union {
    unsigned int full;
    struct {
        unsigned int low : 16;
        unsigned int high : 16;
    } parts;
};

/* Variable with alignment specification */
int aligned_var __attribute__((aligned(64)));

/* String literal pointer - may trigger string attributes */
const char *string_literal = "Another test string";

/* Multi-dimensional array with different storage */
static int multi_array[3][4][5];

/* Function implementations */
int prototyped_function(int a, char b, double c) {
    return a + (int)b + (int)c;
}

int mixed_params(const char *str, volatile int *ptr, 
                 struct small_packed *sp, 
                 int (*callback)(int, int)) {
    static int local_static = 0;
    int local_auto = *ptr;
    
    if (str[0] && sp && callback) {
        local_auto = callback(local_auto, sp->flag1);
    }
    
    local_static++;
    return local_auto + local_static;
}

/* OpenMP parallel region (if supported) */
#ifdef _OPENMP
void omp_test_function(void) {
    int local_omp = 0;
    
    #pragma omp parallel private(local_omp)
    {
        local_omp = omp_get_thread_num();
        omp_thread_var = local_omp;
    }
}
#endif

/* Main function using all types */
int main(void) {
    /* Declare various local variables */
    fixed_string local_string = "local";
    struct picture_data pic_local = {{0}};
    struct optional_data opt = {NULL, 42};
    struct mutable_sim mut = {1, 2};
    struct small_packed sp = {1, 0, 1, 0};
    complex_type ct = {"complex", 1, &pic_local};
    struct outer out = {10, {20, 'X', {30}}, 40.0};
    union bit_union bu = {0x12345678};
    
    /* Use segment pointers if available */
    #ifdef __i386__
    int fs_val;
    fs_segment_ptr = &fs_val;
    #endif
    
    /* Use the variables to prevent optimization */
    volatile_var = static_var + const_var;
    
    if (func_ptr) {
        volatile_var = func_ptr(1, 'a', 3.14);
    }
    
    /* Use arrays */
    for (int i = 0; i < 5; i++) {
        for (int j = 0; j < 5; j++) {
            column_major_array[i][j] = i * j;
        }
    }
    
    /* Call function with mixed params */
    int result = mixed_params(string_literal, &volatile_var, &sp, 
                             prototyped_function);
    
    /* Use aligned variable */
    aligned_var = result * 2;
    
    /* Use union */
    bu.parts.high = 0xABCD;
    
    #ifdef _OPENMP
    omp_test_function();
    #endif
    
    return aligned_var > 0 ? 0 : 1;
}
