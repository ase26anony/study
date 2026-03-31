/* test_dwarf_attrs.c - Comprehensive test for triggering specific DWARF attributes */
/* Compile with: gcc -g -dA -O0 -fopenmp -std=c++17 test_dwarf_attrs.c -o test_dwarf_attrs */

#include <stddef.h>
#include <stdint.h>

/* For DW_AT_string_length, DW_AT_string_length_bit_size, DW_AT_string_length_byte_size */
/* Fixed-length string type that might trigger string length attributes */
typedef char fixed_string[32];
fixed_string global_string = "test_string_for_dwarf";

/* Struct with string member that might have length attributes */
struct string_struct {
    char *ptr;
    char array[64];
    int length;
};

/* For DW_AT_picture_string - COBOL/Fortran PICTURE data simulation */
/* Use GCC's attribute if available */
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
#include <omp.h>
int omp_thread_var = 0;
#pragma omp threadprivate(omp_thread_var)

/* Thread-local storage */
__thread int thread_local_var = 42;
#endif

/* For DW_AT_explicit - C++ explicit constructor (in C we simulate with attributes) */
/* We'll handle this in C++ section below */

/* For DW_AT_is_optional - Simulate optional parameters/variables */
struct optional_data {
    int is_present;
    union {
        int value;
        char padding[sizeof(int)];
    };
};

/* For DW_AT_mutable - C++ mutable (simulated in C with volatile) */
struct mutable_sim {
    volatile int changeable;
    const int constant;
};

/* For DW_AT_ordering - Array ordering (column-major for Fortran) */
#ifdef __GNUC__
int column_major_array[5][5] __attribute__((aligned(16)));
#else
int column_major_array[5][5];
#endif

/* For DW_AT_segment - Segment-specific pointers */
/* Use compiler-specific segment attributes if available */
#ifdef __i386__
#ifdef __GNUC__
int __seg_fs *fs_seg_ptr = 0;
int __seg_gs *gs_seg_ptr = 0;
#endif
#endif

/* For DW_AT_prototyped - Function with prototype */
int prototyped_function(int a, char b, double c);
int (*func_ptr)(int, char, double) = &prototyped_function;

/* For DW_AT_small - Packed small types */
struct small_packed {
    unsigned int flag:1;
    unsigned int tiny:3;
    unsigned int :4; /* padding */
} __attribute__((packed));

/* Complex type compositions to stress DIE generation */
typedef int (*complex_func_ptr)(struct string_struct*, const struct picture_data*);
typedef volatile const int * restrict volatile_ptr;

/* Storage class variations */
static int static_var = 100;
register int register_var asm("ebx"); /* Note: compiler may ignore or reassign */
const int const_global = 200;
volatile int volatile_global = 300;
restrict int *restrict_ptr = 0;

/* Function definitions */
int prototyped_function(int a, char b, double c) {
    return a + (int)b + (int)c;
}

/* Function with various local variables */
void test_function(void) {
    /* Local fixed string */
    fixed_string local_str = "local_test";
    
    /* Picture data local */
    struct picture_data local_pic = {{0}};
    
    /* Small packed struct */
    struct small_packed local_small = {0};
    
    /* Mutable simulation */
    struct mutable_sim local_mut = {0, 1};
    
    /* Optional data */
    struct optional_data local_opt = {0};
    
    /* Array with potential ordering */
    int local_array[3][3] = {{1,2,3},{4,5,6},{7,8,9}};
    
    /* Complex pointer */
    complex_func_ptr local_complex_ptr = 0;
    
    /* Use variables to avoid optimization */
    static_var += local_str[0];
    if (volatile_global) {
        local_mut.changeable = static_var;
    }
    
#ifdef _OPENMP
    #pragma omp parallel
    {
        omp_thread_var = omp_get_thread_num();
        thread_local_var = omp_thread_var * 10;
    }
#endif
}

/* Main function */
int main(void) {
    test_function();
    
    /* Use global variables */
    global_string[0] = 'A';
    
#ifdef __i386__
#ifdef __GNUC__
    /* Try to use segment pointers if available */
    if (fs_seg_ptr) {
        /* Access through segment pointer */
    }
#endif
#endif
    
    /* Call through function pointer */
    if (func_ptr) {
        int result = func_ptr(1, 'a', 3.14);
        return result > 0 ? 0 : 1;
    }
    
    return 0;
}

/* Additional declarations at file scope for more DIEs */
extern int external_var;
int defined_external_var = 500;

/* Union for complex type */
union complex_union {
    int i;
    float f;
    char c[4];
    struct {
        unsigned int a:8;
        unsigned int b:8;
        unsigned int c:8;
        unsigned int d:8;
    } bits;
};

/* Typedef chain */
typedef int base_type;
typedef base_type derived_type;
typedef derived_type *pointer_to_derived;
typedef pointer_to_derived array_of_pointers[10];

/* Enum for additional debug info */
enum debug_enum {
    DEBUG_ATTR_NONE,
    DEBUG_ATTR_STRING_LENGTH,
    DEBUG_ATTR_PICTURE,
    DEBUG_ATTR_THREADS,
    DEBUG_ATTR_COUNT
};
