/*
 * Test program to trigger specific DWARF attribute generation in GCC
 * Each section targets specific DWARF_AT_* attributes
 */

#include <stddef.h>
#include <stdint.h>

/* For DW_AT_string_length, DW_AT_string_length_bit_size, DW_AT_string_length_byte_size */
/* Fixed-length string type - may generate string length attributes */
typedef char fixed_string_t[32];
fixed_string_t global_string = "test_string";

/* Struct with string member - potential for string length attributes */
struct string_struct {
    char *dynamic_str;
    char fixed_str[64];
};

/* For DW_AT_picture_string - COBOL/Fortran style picture data */
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
__thread int tls_var = 42;
#endif

/* For DW_AT_explicit - C++ explicit constructor (in C++ section) */

/* For DW_AT_is_optional - std::optional (in C++ section) */

/* For DW_AT_mutable - mutable member (in C++ section) */

/* For DW_AT_ordering - array ordering */
#ifdef __GNUC__
int column_major_array[5][5] __attribute__((column_major));
#else
int column_major_array[5][5];
#endif

/* For DW_AT_segment - segment-specific pointers */
#ifdef __i386__
/* x86 specific segment registers */
int * __attribute__((segment("fs"))) fs_ptr = NULL;
int * __attribute__((segment("gs"))) gs_ptr = NULL;
#elif defined(__x86_64__)
/* x86-64 uses FS/GS for thread-local storage */
int * __attribute__((segment("fs"))) fs_ptr = NULL;
#endif

/* For DW_AT_prototyped - function with prototype */
int prototyped_function(int a, char b, double c);
int (*func_ptr)(int, char, double) = &prototyped_function;

/* For DW_AT_small - packed small types */
struct small_struct {
    unsigned int flag:1;
    unsigned int tiny:3;
} __attribute__((packed));

/* For DW_AT_lower_bound - array with non-zero lower bound */
/* Use GNU extension for array ranges */
#ifdef __GNUC__
int ranged_array[10] __attribute__((aligned(16)));
#else
int ranged_array[10];
#endif

/* For DW_AT_location - variables with complex locations */
register int reg_var asm("ebx");  /* May not work on all targets */

/* Complex type compositions to stress DIE generation */
typedef int (*complex_func_t)(int, ...);
typedef struct {
    const volatile int *restrict ptr;
    int array[10];
} nested_struct_t;

/* Storage class variations */
static int static_var = 100;
extern int extern_var;
volatile int volatile_var = 200;
const int const_var = 300;
restrict int *restrict_ptr = NULL;

/* Function definitions */
int prototyped_function(int a, char b, double c) {
    return a + (int)b + (int)c;
}

/* Main function with various scopes */
int main(int argc, char *argv[]) {
    /* Local variables in different scopes */
    {
        /* Block scope variables */
        auto int auto_var = 10;
        register int local_reg = 20;
        
        /* String variables - potential for string length attributes */
        char local_string[] = "local_string";
        fixed_string_t local_fixed = "fixed_local";
        
        /* Struct instances */
        struct string_struct str_inst = {
            .dynamic_str = "dynamic",
            .fixed_str = "fixed"
        };
        
        /* Picture data instance */
        struct picture_data pic_inst = { .data = "1234567890" };
        
        /* Small struct instance */
        struct small_struct small_inst = { .flag = 1, .tiny = 5 };
        
        /* Use all variables to prevent optimization */
        (void)auto_var;
        (void)local_reg;
        (void)local_string;
        (void)local_fixed;
        (void)str_inst;
        (void)pic_inst;
        (void)small_inst;
    }
    
#ifdef _OPENMP
    /* OpenMP section for threads_scaled */
    #pragma omp parallel
    {
        omp_thread_var = omp_get_thread_num();
        tls_var = omp_get_thread_num() * 100;
    }
#endif
    
    /* Use global variables */
    (void)global_string;
    (void)column_major_array;
    (void)func_ptr;
    (void)ranged_array;
    (void)static_var;
    (void)volatile_var;
    (void)const_var;
    (void)restrict_ptr;
    
    return 0;
}

/* Additional global variables */
int extern_var = 400;

/* Complex nested type */
struct outer_struct {
    struct {
        int inner_a;
        char inner_b;
    } nested;
    union {
        int u_a;
        float u_b;
    } data_union;
};

/* Variable with alignment requirement */
int __attribute__((aligned(64))) aligned_var = 500;

/* Bitfield structure */
struct bitfield_struct {
    unsigned int a:4;
    unsigned int b:8;
    unsigned int c:12;
    unsigned int d:8;
};

/* Array of function pointers */
int (*func_array[5])(int, char, double);

/* Forward declaration for incomplete type */
struct incomplete_struct;
struct incomplete_struct *incomplete_ptr = NULL;
