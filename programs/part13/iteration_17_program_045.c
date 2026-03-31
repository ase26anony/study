/*
 * Test program to trigger specific DWARF attribute generation in GCC
 * Each section targets specific DWARF_AT_* attributes
 */

#include <stddef.h>

/* For DW_AT_string_length, DW_AT_string_length_bit_size, DW_AT_string_length_byte_size */
/* Fixed-length string type - may generate string length attributes */
typedef char fixed_string[32];
fixed_string global_string = "test_string";

/* Struct with string member - may trigger string length attributes */
struct string_struct {
    char data[64];
    int length;
};

/* For DW_AT_picture_string - COBOL/Fortran PICTURE data simulation */
/* Use GCC's attribute if available */
#ifdef __GNUC__
struct picture_data {
    char value[20];
} __attribute__((packed));
#else
#pragma pack(push, 1)
struct picture_data {
    char value[20];
};
#pragma pack(pop)
#endif

/* For DW_AT_threads_scaled - OpenMP thread-related variables */
#ifdef _OPENMP
#include <omp.h>
int omp_thread_var = 0;
#pragma omp threadprivate(omp_thread_var)
#endif

/* For DW_AT_explicit - C++ explicit constructor (in C++ section) */

/* For DW_AT_is_optional - C++17 std::optional (in C++ section) */

/* For DW_AT_mutable - C++ mutable member (in C++ section) */

/* For DW_AT_ordering - Column-major array ordering */
#ifdef __GNUC__
int column_major_array[5][5] __attribute__((column_major));
#else
int column_major_array[5][5];
#endif

/* For DW_AT_segment - Segment-specific pointers */
#ifdef __i386__
/* x86 segment register pointers */
int __seg_fs *fs_pointer = NULL;
int __seg_gs *gs_pointer = NULL;
#endif

/* For DW_AT_prototyped - Function with prototype */
int prototyped_function(int a, char b);
int (*function_pointer)(int, char) = &prototyped_function;

/* For DW_AT_small - Packed struct with bit-field */
struct small_struct {
    unsigned int flag:1;
    unsigned int value:7;
} __attribute__((packed));

/* For DW_AT_location - Variable with complex location */
register int reg_var asm("ebx");

/* For DW_AT_lower_bound - Array with specified bounds */
int bounded_array[10] = {0};

/* Additional stress cases with various storage classes and qualifiers */
static const volatile int static_const_volatile = 42;
thread_local int tls_var = 100;
restrict int *restrict_ptr = NULL;

/* Complex type compositions */
typedef int (*complex_func_ptr)(int (*)(char), void*);
complex_func_ptr complex_ptr = NULL;

/* Function definitions */
int prototyped_function(int a, char b) {
    return a + (int)b;
}

/* Main function with various local variables */
int main(int argc, char *argv[]) {
    /* Local string - may trigger string length attributes */
    char local_string[50] = "local_test";
    
    /* Local struct instances */
    struct string_struct local_str_struct = {"hello", 5};
    struct small_struct local_small = {1, 42};
    
    /* Array with different storage */
    static int static_array[20];
    
    /* Pointer with segment attribute (if supported) */
    #ifdef __i386__
    int __seg_fs *local_fs_ptr = fs_pointer;
    #endif
    
    /* Use OpenMP if available */
    #ifdef _OPENMP
    #pragma omp parallel
    {
        omp_thread_var = omp_get_thread_num();
    }
    #endif
    
    /* Use all variables to prevent optimization */
    int sum = 0;
    sum += global_string[0];
    sum += local_string[0];
    sum += local_str_struct.length;
    sum += local_small.flag;
    sum += column_major_array[0][0];
    sum += static_const_volatile;
    sum += tls_var;
    sum += bounded_array[0];
    sum += argc;
    
    return sum;
}

/* Additional file-scope variables for more DIEs */
extern int external_var;
const double pi = 3.14159;
volatile int signal_flag = 0;

/* Union for type variety */
union data_union {
    int i;
    float f;
    char str[4];
};

/* Enum for enumeration type DIE */
enum color {
    RED,
    GREEN,
    BLUE
};

/* Typedef chain for complex type DIEs */
typedef int base_type;
typedef base_type *pointer_type;
typedef pointer_type array_of_pointers[10];
