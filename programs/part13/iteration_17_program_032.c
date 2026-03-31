/* Test program to trigger specific DWARF attribute generation in GCC */
/* This file contains constructs designed to trigger various DWARF attributes */

#include <stddef.h>
#include <stdint.h>

/* For DW_AT_string_length, DW_AT_string_length_bit_size, DW_AT_string_length_byte_size */
/* Fixed-length string type - may trigger string length attributes */
typedef char fixed_string_t[32];
fixed_string_t global_string = "test_string_for_dwarf";

/* For DW_AT_picture_string - COBOL/Fortran PICTURE data simulation */
#ifdef __GNUC__
/* Use GCC attributes for picture strings if supported */
struct picture_data {
    char data[20];
} __attribute__((picture("9(5)V9(2)")));
#else
struct picture_data {
    char data[20];
};
#endif

/* For DW_AT_threads_scaled - OpenMP thread-related variables */
#ifdef _OPENMP
#include <omp.h>
int omp_thread_var = 0;
#pragma omp threadprivate(omp_thread_var)

/* Thread-local storage */
__thread int thread_local_var = 42;
#endif

/* For DW_AT_segment - Segment-specific pointers */
#ifdef __i386__
/* x86 segment registers */
int __seg_fs *fs_seg_ptr = 0;
int __seg_gs *gs_seg_ptr = 0;
#endif

/* For DW_AT_prototyped - Function with prototype */
int prototyped_function(int a, char b) {
    return a + b;
}

/* Function pointer with prototype */
int (*proto_func_ptr)(int, char) = &prototyped_function;

/* For DW_AT_small - Packed structure with bit-fields */
struct small_packed_struct {
    unsigned int flag1:1;
    unsigned int flag2:1;
    unsigned int value:6;
} __attribute__((packed));

/* For DW_AT_ordering - Column-major array ordering */
#ifdef __GNUC__
int column_major_array[5][5] __attribute__((column_major));
#else
int column_major_array[5][5];
#endif

/* Complex type compositions to stress DIE generation */
typedef struct {
    fixed_string_t name;
    int id;
    struct small_packed_struct flags;
} complex_type_t;

/* Variable with various storage classes and qualifiers */
static const volatile int static_const_volatile = 100;
register int register_var asm("ebx");  /* May not work on all targets */

/* Array of function pointers */
typedef int (*func_ptr_t)(int, char);
func_ptr_t func_array[3] = {&prototyped_function, NULL, NULL};

/* Nested structure with arrays */
struct nested_struct {
    int matrix[3][3];
    struct {
        char tag;
        int data;
    } inner;
    const char *const_string;
};

/* Union with bit-fields */
union bitfield_union {
    struct {
        unsigned int a:4;
        unsigned int b:4;
        unsigned int c:8;
    } bits;
    uint16_t value;
};

/* Variable with restrict qualifier */
void use_restrict(int *restrict ptr1, int *restrict ptr2) {
    *ptr1 = *ptr2 + 1;
}

/* Inline function with debug info */
static inline int inline_debug_func(int x) __attribute__((always_inline));
static inline int inline_debug_func(int x) {
    return x * 2;
}

/* Main function with local variables */
int main(int argc, char *argv[]) {
    /* Local fixed string */
    fixed_string_t local_string = "local_test";
    
    /* Picture data instance */
    struct picture_data pic_var = {{0}};
    
    /* Small packed struct */
    struct small_packed_struct small_var = {1, 0, 42};
    
    /* Complex type instance */
    complex_type_t complex_var = {"complex", 123, {1, 1, 63}};
    
    /* Nested struct */
    struct nested_struct nested_var = {
        {{1,2,3},{4,5,6},{7,8,9}},
        {'A', 999},
        "nested_string"
    };
    
    /* Union */
    union bitfield_union union_var = {{1, 2, 3}};
    
    /* Array with ordering */
    for (int i = 0; i < 5; i++) {
        for (int j = 0; j < 5; j++) {
            column_major_array[i][j] = i * 5 + j;
        }
    }
    
#ifdef _OPENMP
    /* OpenMP parallel region */
    #pragma omp parallel
    {
        omp_thread_var = omp_get_thread_num();
        thread_local_var = omp_get_thread_num() * 10;
    }
#endif
    
    /* Use function pointer */
    int result = proto_func_ptr(10, 'A');
    
    /* Use inline function */
    int doubled = inline_debug_func(argc);
    
    /* Use restrict */
    int a = 1, b = 2;
    use_restrict(&a, &b);
    
    /* Return computed value to prevent optimization */
    return result + doubled + a + b + 
           local_string[0] + pic_var.data[0] + small_var.value +
           complex_var.id + nested_var.inner.data + union_var.value;
}

/* Additional file-scope variables with different attributes */
volatile int *volatile volatile_ptr = (volatile int*)0x1000;
const struct nested_struct *const const_struct_ptr = NULL;

/* Array of complex types */
complex_type_t complex_array[10];

/* Function returning pointer to array */
int (*func_returning_array_ptr(int size))[10] {
    static int array[10];
    return &array;
}

/* Variable with asm label */
int asm_label_var asm("special_var") = 777;

/* Weak symbol */
int weak_var __attribute__((weak)) = 888;

/* Aligned variable */
int aligned_var __attribute__((aligned(64))) = 999;

/* Used attribute to prevent optimization */
int used_var __attribute__((used)) = 1111;
