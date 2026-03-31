/* Test program to trigger specific DWARF attribute generation in GCC */
/* Compile with: gcc -g -dA -O0 test_dwarf_attrs.c -o test_dwarf_attrs */

#include <stddef.h>
#include <stdint.h>

/* For DW_AT_string_length, DW_AT_string_length_bit_size, DW_AT_string_length_byte_size */
/* Fixed-length string type that might generate string length attributes */
typedef char fixed_string[32];
fixed_string global_string = "test_string_for_dwarf";

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
#endif

/* For DW_AT_segment - Segment-specific pointers (x86 memory models) */
#ifdef __i386__
int * __attribute__((segment("fs"))) fs_seg_ptr = 0;
int * __attribute__((segment("gs"))) gs_seg_ptr = 0;
#endif

/* For DW_AT_prototyped - Function with prototype */
int prototyped_function(int a, char b) {
    return a + b;
}

/* Function pointer to prototyped function */
int (*proto_ptr)(int, char) = &prototyped_function;

/* For DW_AT_small - Packed structure with bit-field */
struct small_struct {
    unsigned int flag:1;
    unsigned int tiny:3;
} __attribute__((packed));

/* For DW_AT_ordering - Column-major array (Fortran-style) */
#ifdef __GNUC__
int column_major_array[5][5] __attribute__((column_major));
#else
int column_major_array[5][5];
#endif

/* Complex type compositions to stress DIE generation */
typedef struct {
    fixed_string name;
    int id;
    struct small_struct flags;
} complex_type;

/* Variables with different storage classes */
static int static_var = 42;
volatile int volatile_var = 100;
const int const_var = 200;
register int register_var asm("eax");

/* Thread-local storage */
_Thread_local int thread_local_var = 300;

/* Restrict pointer */
void use_restrict(int *restrict ptr1, int *restrict ptr2) {
    *ptr1 += *ptr2;
}

/* Array of function pointers */
typedef int (*func_ptr_t)(int, char);
func_ptr_t func_array[3] = {&prototyped_function, NULL, NULL};

/* Nested structures */
struct outer {
    struct inner {
        int x;
        int y;
    } nested;
    int z;
};

/* Union with bit-fields */
union bit_union {
    struct {
        unsigned int a:8;
        unsigned int b:8;
        unsigned int c:8;
        unsigned int d:8;
    } fields;
    uint32_t value;
};

/* Variable with alignment specification */
int aligned_var __attribute__((aligned(64)));

/* Main function using all variables */
int main(int argc, char *argv[]) {
    /* Local fixed string */
    fixed_string local_string = "local_test";
    
    /* Picture data */
    struct picture_data pic = {"12345678901234567890"};
    
    /* Small struct */
    struct small_struct small = {1, 7};
    
    /* Complex type */
    complex_type ct = {"complex", 1, {1, 5}};
    
    /* Use column major array */
    for (int i = 0; i < 5; i++) {
        for (int j = 0; j < 5; j++) {
            column_major_array[i][j] = i * 5 + j;
        }
    }
    
    /* Use OpenMP if available */
    #ifdef _OPENMP
    #pragma omp parallel
    {
        omp_thread_var = omp_get_thread_num();
    }
    #endif
    
    /* Use segment pointers if available */
    #ifdef __i386__
    /* These would need proper initialization in real code */
    (void)fs_seg_ptr;
    (void)gs_seg_ptr;
    #endif
    
    /* Use all variables to prevent optimization */
    (void)global_string;
    (void)local_string;
    (void)pic;
    (void)small;
    (void)ct;
    (void)static_var;
    (void)volatile_var;
    (void)const_var;
    (void)thread_local_var;
    (void)aligned_var;
    (void)func_array;
    
    /* Call prototyped function */
    int result = prototyped_function(10, 'A');
    
    /* Use restrict */
    int x = 5, y = 10;
    use_restrict(&x, &y);
    
    return result + argc;
}

/* Additional test functions */
void test_vla(int size) {
    /* Variable Length Array - generates interesting debug info */
    int vla[size];
    for (int i = 0; i < size; i++) {
        vla[i] = i;
    }
}

/* Inline function */
static inline int inline_func(int x) {
    return x * 2;
}

/* Weak symbol */
__attribute__((weak)) int weak_var = 0;

/* No-return function */
__attribute__((noreturn)) void exit_program(void) {
    _Exit(0);
}
