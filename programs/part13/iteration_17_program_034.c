/* Test program to trigger various DWARF attribute generation in GCC */
/* Compile with: gcc -g -dA -O0 test_dwarf_attrs.c -o test_dwarf_attrs */

#include <stddef.h>
#include <string.h>

/* For DW_AT_string_length, DW_AT_string_length_bit_size, DW_AT_string_length_byte_size */
/* Fixed-length string type that might trigger string length attributes */
typedef char fixed_string[32];
fixed_string global_string = "Hello, DWARF!";

/* Struct with string member - might trigger string-related attributes */
struct string_struct {
    char *dynamic_string;
    char fixed_array[64];
    int length;
};

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
#include <omp.h>
int omp_thread_var;
#pragma omp threadprivate(omp_thread_var)
#endif

/* For DW_AT_explicit - C++ explicit constructor (in C we simulate with attributes) */
/* This will be handled in the C++ version */

/* For DW_AT_is_optional - simulate optional parameters */
struct optional_data {
    int is_present;
    union {
        int value;
        char padding[sizeof(int)];
    } data;
};

/* For DW_AT_mutable - C++ mutable (simulated in C) */
struct mutable_sim {
    int regular;
    int changeable; /* Simulating mutable */
};

/* For DW_AT_ordering - array ordering (column-major for Fortran) */
#ifdef __GNUC__
int column_major_array[5][5] __attribute__((aligned(16)));
#else
int column_major_array[5][5];
#endif

/* For DW_AT_segment - segment-specific pointers */
/* Use compiler-specific segment attributes if available */
#ifdef __i386__
#ifdef __GNUC__
int __seg_fs *fs_seg_ptr;
int __seg_gs *gs_seg_ptr;
#endif
#endif

/* For DW_AT_prototyped - function with prototype */
int prototyped_function(int a, char b, double c);
int (*func_ptr)(int, char, double) = &prototyped_function;

/* For DW_AT_small - packed small types */
struct small_packed {
    unsigned int flag:1;
    unsigned int tiny:3;
    unsigned int :4; /* padding */
} __attribute__((packed));

/* Complex type compositions to stress DIE generation */
typedef int (*complex_func_ptr)(struct string_struct*, const struct picture_data*);
typedef volatile const int* restrict volatile_restrict_ptr;

/* Storage class variations */
static int static_var = 42;
#ifdef __STDC_VERSION__
#if __STDC_VERSION__ >= 201112L
_Thread_local int thread_local_var = 100;
#endif
#endif
register int register_var asm("ebx"); /* Target-specific */

/* Function declarations with various attributes */
#ifdef __GNUC__
void __attribute__((noinline)) noinline_func(int x) {
    static_var = x;
}

int __attribute__((const)) const_func(int a) {
    return a * 2;
}
#endif

/* Prototyped function implementation */
int prototyped_function(int a, char b, double c) {
    return a + (int)b + (int)c;
}

/* Function with complex debug info requirements */
struct string_struct create_string_struct(const char* str) {
    struct string_struct s;
    s.length = (int)strlen(str);
    
    /* Simulate string copy */
    for (int i = 0; i < s.length && i < 63; i++) {
        s.fixed_array[i] = str[i];
    }
    s.fixed_array[s.length < 63 ? s.length : 63] = '\0';
    
    return s;
}

/* Main function with various scopes and variables */
int main(int argc, char *argv[]) {
    /* Local variables with different storage classes */
    auto int auto_var = 10;
    volatile int volatile_var = 20;
    const int const_var = 30;
    
    /* String-related variables */
    fixed_string local_string = "Local string";
    struct string_struct str_obj = create_string_struct("Test");
    
    /* Picture data */
    struct picture_data pic = { .data = "9999999999" };
    
    /* Optional data */
    struct optional_data opt = { .is_present = 1, .data.value = 99 };
    
    /* Mutable simulation */
    struct mutable_sim mut = { .regular = 1, .changeable = 2 };
    mut.changeable = 3; /* Simulating mutable modification */
    
    /* Array with potential column-major ordering */
    for (int i = 0; i < 5; i++) {
        for (int j = 0; j < 5; j++) {
            column_major_array[i][j] = i * 5 + j;
        }
    }
    
    /* Small packed struct */
    struct small_packed small = { .flag = 1, .tiny = 5 };
    
    /* Complex pointers */
    int* restrict restrict_ptr = &auto_var;
    volatile_restrict_ptr complex_ptr = &volatile_var;
    
    /* Function pointer usage */
    int result = func_ptr(10, 'A', 3.14);
    
    /* OpenMP section if available */
    #ifdef _OPENMP
    #pragma omp parallel
    {
        omp_thread_var = omp_get_thread_num();
    }
    #endif
    
    /* Register variable simulation */
    int local_register __asm__("eax") = 50;
    (void)local_register;
    
    /* Use all variables to prevent optimization */
    return auto_var + volatile_var + const_var + 
           str_obj.length + opt.data.value + mut.changeable +
           column_major_array[0][0] + small.flag + result +
           static_var;
}

/* Additional global variables for more coverage */
extern int external_var;
int* pointer_array[10];
const double const_array[] = {1.0, 2.0, 3.0, 4.0};

/* Union for variant type debugging */
union variant_type {
    int int_val;
    float float_val;
    char char_val;
    void* ptr_val;
};

/* Enum with explicit values */
enum debug_enum {
    DEBUG_ATTR_NONE = 0,
    DEBUG_ATTR_STRING = 1,
    DEBUG_ATTR_PICTURE = 2,
    DEBUG_ATTR_THREAD = 3,
    DEBUG_ATTR_EXPLICIT = 4,
    DEBUG_ATTR_OPTIONAL = 5,
    DEBUG_ATTR_MUTABLE = 6,
    DEBUG_ATTR_ORDERING = 7,
    DEBUG_ATTR_SEGMENT = 8,
    DEBUG_ATTR_PROTOTYPED = 9,
    DEBUG_ATTR_SMALL = 10
};
