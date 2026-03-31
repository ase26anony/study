/*
 * Test program to trigger generation of specific DWARF attributes
 * in GCC's dwarf2out.cc (lines 7602-7643)
 */

#include <stddef.h>
#include <stdint.h>

/* For DW_AT_string_length, DW_AT_string_length_bit_size, DW_AT_string_length_byte_size */
/* Fixed-length string type (simulates Fortran CHARACTER) */
typedef char fixed_string_t[32];
fixed_string_t global_string = "test_string";

/* String with explicit length (common in database/Cobol interfaces) */
struct string_with_len {
    int32_t length;
    char data[1];
} __attribute__((packed));

/* For DW_AT_picture_string - COBOL-style picture clause simulation */
#ifdef __GNUC__
/* GCC extension for COBOL compatibility */
struct cobol_picture {
    char digits[10];
} __attribute__((picture("9(5)V9(2)")));
#else
/* Fallback for compilers without picture attribute */
struct cobol_picture {
    char digits[10];
};
#endif

/* For DW_AT_threads_scaled - OpenMP thread-related variables */
#ifdef _OPENMP
#include <omp.h>
int omp_thread_var;
#pragma omp threadprivate(omp_thread_var)

/* Thread-local storage */
__thread int thread_local_var = 42;
#endif

/* For DW_AT_explicit - C++ explicit constructor (in C we simulate with attributes) */
#ifdef __cplusplus
class ExplicitClass {
public:
    explicit ExplicitClass(int x) : value(x) {}
private:
    int value;
};
#endif

/* For DW_AT_is_optional - Simulated optional type */
struct optional_int {
    int has_value;
    int value;
};

/* For DW_AT_mutable - C++ mutable (simulated in C) */
#ifdef __cplusplus
class MutableClass {
public:
    MutableClass() : counter(0) {}
    void increment() const { counter++; }  // Can modify mutable member
private:
    mutable int counter;
};
#endif

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
int prototyped_function(int a, char b, double c);
int (*func_ptr)(int, char, double) = &prototyped_function;

/* For DW_AT_small - Packed small type */
struct small_packed {
    unsigned int flag : 1;
    unsigned int tiny : 3;
} __attribute__((packed));

/* Complex type compositions to stress DIE generation */
typedef struct {
    fixed_string_t name;
    struct optional_int opt_data;
    volatile int * volatile ptr;
    const struct small_packed * const_packed;
} complex_type_t;

/* Variables with different storage classes */
static int static_var = 100;
register int reg_var asm("ebx");  /* May not work on all targets */
volatile int volatile_var;
const int const_var = 200;
restrict int *restrict_ptr;

/* Function with mixed parameters */
void complex_func(
    const char *str,
    volatile int *vol_ptr,
    struct string_with_len *str_with_len,
    int arr[static 10]
) {
    /* Local variables with different attributes */
    auto int auto_var = 50;
    static int func_static = 0;
    
    /* Array with different ordering hint */
    int local_array[3][3];
    
    /* Use all variables to prevent optimization */
    if (str) auto_var++;
    if (vol_ptr) (*vol_ptr)++;
    if (str_with_len) str_with_len->length++;
    if (arr) arr[0] = auto_var;
    
    func_static++;
}

/* Prototyped function implementation */
int prototyped_function(int a, char b, double c) {
    return a + (int)b + (int)c;
}

/* Main function to use all constructs */
int main(int argc, char *argv[]) {
    /* Initialize and use all global variables */
    complex_type_t ct = {
        .name = "test",
        .opt_data = {1, 42},
        .ptr = &volatile_var,
        .const_packed = NULL
    };
    
    /* Use string types */
    fixed_string_t local_string;
    for (int i = 0; i < 31; i++) {
        local_string[i] = 'A' + (i % 26);
    }
    local_string[31] = '\0';
    
    /* Use array with potential column-major ordering */
    for (int i = 0; i < 5; i++) {
        for (int j = 0; j < 5; j++) {
            column_major_array[i][j] = i * 5 + j;
        }
    }
    
    /* Use small packed structure */
    struct small_packed sp = {1, 7};
    
    /* Call complex function */
    int local_arr[10] = {0};
    complex_func(local_string, &volatile_var, NULL, local_arr);
    
    /* Use function pointer */
    int result = func_ptr(10, 'X', 3.14);
    
    /* Thread-related code */
#ifdef _OPENMP
    #pragma omp parallel
    {
        omp_thread_var = omp_get_thread_num();
        thread_local_var = omp_get_thread_num() * 100;
    }
#endif
    
    return result + static_var + const_var + sp.flag + sp.tiny;
}

/* Additional type with bitfields for string length attributes */
struct db_string {
    unsigned int length : 16;    /* 16-bit length field */
    unsigned int capacity : 16;  /* 16-bit capacity field */
    char *data;
};

/* Union with different representations */
union multi_rep {
    struct {
        int type : 4;
        int value : 28;
    } bits;
    int32_t raw;
    float as_float;
};

/* Variable with alignment requirement */
int __attribute__((aligned(64))) aligned_var;

/* Recursive type definition */
struct tree_node {
    int value;
    struct tree_node *left;
    struct tree_node *right;
};

/* External linkage test */
extern int external_var;

/* Weak symbol */
int __attribute__((weak)) weak_var = 0;

/* Section-specific variable */
int __attribute__((section(".mysection"))) section_var = 999;

/* Cleanup attribute */
void cleanup_func(int *p) {
    if (p) *p = 0;
}

/* Transparent union */
typedef union {
    int *int_ptr;
    void *void_ptr;
} transparent_union_t __attribute__((transparent_union));
