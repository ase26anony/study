/* Test program to trigger various DWARF attribute generation in GCC */
/* Compile with: gcc -g -dA -O0 -fopenmp -std=c++17 test_dwarf_attrs.c -o test_dwarf_attrs */

#include <stddef.h>
#include <stdint.h>

/* For DW_AT_string_length, DW_AT_string_length_bit_size, DW_AT_string_length_byte_size */
/* Fixed-length string type that might trigger string length attributes */
typedef char fixed_string[32];
fixed_string global_string = "test_string_for_dwarf";

/* Structure with string member */
struct string_struct {
    char *dynamic_string;
    fixed_string fixed_str;
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
int omp_global_var = 0;
#pragma omp threadprivate(omp_global_var)

/* Thread-local storage */
__thread int thread_local_var = 42;
#endif

/* For DW_AT_explicit - C++ explicit constructor (in C we simulate with function attributes) */
/* We'll use a separate C++ file for actual C++ features */

/* For DW_AT_is_optional - simulate optional parameters */
struct optional_param {
    int is_present;
    union {
        int int_value;
        void *ptr_value;
    } data;
};

/* For DW_AT_mutable - C++ mutable (simulated in C with volatile) */
struct mutable_sim {
    volatile int changeable;
    const int constant;
};

/* For DW_AT_ordering - array ordering attributes */
#ifdef __GNUC__
int column_major_array[5][5] __attribute__((aligned(16)));
#else
int column_major_array[5][5];
#endif

/* For DW_AT_segment - segment-specific pointers (x86 specific) */
#ifdef __i386__
#ifdef __GNUC__
int * __attribute__((segment("fs"))) fs_seg_ptr;
int * __attribute__((segment("gs"))) gs_seg_ptr;
#endif
#endif

/* For DW_AT_prototyped - function with prototype */
int prototyped_function(int a, char b, double c);
int (*func_ptr)(int, char, double) = &prototyped_function;

/* For DW_AT_small - packed structure with bitfields */
struct small_packed {
    unsigned int flag1:1;
    unsigned int flag2:1;
    unsigned int flag3:1;
    unsigned int flag4:1;
    unsigned int value:4;
} __attribute__((packed));

/* Complex type compositions to stress DIE generation */
typedef int (*complex_func_ptr)(struct string_struct *, const struct picture_data *);
typedef volatile const int * restrict volatile_ptr;

/* Storage class variations */
static int static_var = 100;
register int register_var asm("ebx");  /* Note: register selection is compiler-dependent */
auto int auto_var = 200;  /* auto is default in C, but explicit for testing */

/* Function declarations with various attributes */
#ifdef __GNUC__
void __attribute__((noinline)) noinline_func(void) {
    static_var++;
}

int __attribute__((const)) const_func(int x) {
    return x * 2;
}
#endif

/* The prototyped function implementation */
int prototyped_function(int a, char b, double c) {
    return a + (int)b + (int)c;
}

/* Main function with various scopes */
int main(int argc, char *argv[]) {
    /* Local variables with different storage classes */
    static int local_static = 0;
    volatile int volatile_var = argc;
    const int const_var = 42;
    int *restrict restrict_ptr = &volatile_var;
    
    /* String variables for string length attributes */
    fixed_string local_string = "local_test";
    char *heap_string = "heap_allocated";
    
    /* Structure instances */
    struct string_struct str_inst = {
        .dynamic_string = heap_string,
        .fixed_str = "struct_string"
    };
    
    struct picture_data pic_inst = {
        .data = "PICTURE1234567890"
    };
    
    struct small_packed small_inst = {
        .flag1 = 1,
        .flag2 = 0,
        .flag3 = 1,
        .flag4 = 0,
        .value = 7
    };
    
    /* Array operations */
    for (int i = 0; i < 5; i++) {
        for (int j = 0; j < 5; j++) {
            column_major_array[i][j] = i * 5 + j;
        }
    }
    
#ifdef _OPENMP
    /* OpenMP section for threads_scaled */
    #pragma omp parallel
    {
        omp_global_var = omp_get_thread_num();
        thread_local_var = omp_get_thread_num() * 10;
    }
#endif
    
    /* Function pointer usage */
    int result = func_ptr(10, 'A', 3.14);
    
    /* Complex pointer usage */
    complex_func_ptr complex_ptr = NULL;
    
    /* Return based on various conditions */
    return static_var + local_static + result + small_inst.value;
}

/* Additional functions for more DIEs */
void helper_function(void) {
    /* Nested block for block-scoped DIEs */
    {
        int block_scoped = 99;
        block_scoped++;
    }
    
    /* Union for variant type DIE */
    union variant {
        int i;
        float f;
        char *s;
    } var_union;
    
    var_union.i = 42;
}

/* Global union */
union global_variant {
    long long ll;
    double d;
    void *p;
} global_union_var;

/* Enum for enum type DIE */
enum color {
    RED,
    GREEN,
    BLUE,
    ALPHA = 255
} color_var = GREEN;

/* Typedef chain */
typedef int my_int;
typedef my_int my_int2;
typedef my_int2 my_int3;

my_int3 typedef_var = 123;

/* External linkage test */
extern int external_var;

/* Weak symbol */
#ifdef __GNUC__
int __attribute__((weak)) weak_var = 0;
#endif

/* Aligned variable */
int __attribute__((aligned(64))) aligned_var[16];

/* Section attribute */
#ifdef __GNUC__
int __attribute__((section(".my_section"))) section_var = 999;
#endif

/* Cleanup attribute */
#ifdef __GNUC__
void cleanup_func(int *p) {
    *p = 0;
}

void test_cleanup(void) {
    int __attribute__((cleanup(cleanup_func))) auto_clean = 100;
    auto_clean = 200;
}
#endif
