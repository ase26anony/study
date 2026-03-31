/* Test program to trigger various DWARF attribute generation in GCC */
/* Compile with: gcc -g -dA -O0 -fopenmp -std=c++17 test_dwarf_attrs.c -o test_dwarf_attrs */

#include <stddef.h>
#include <stdint.h>

/* For DW_AT_string_length, DW_AT_string_length_bit_size, DW_AT_string_length_byte_size */
/* Fixed-length string type that might generate string length attributes */
typedef char fixed_string[32];
fixed_string global_string = "test_string_for_dwarf";

/* Structure with string member */
struct string_struct {
    char *dynamic_string;
    char fixed_array[64];
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
int omp_global_var = 42;
#pragma omp threadprivate(omp_global_var)
#endif

/* For DW_AT_explicit - C++ explicit constructor (in C we simulate with attributes) */
/* We'll use C++ for this part, but provide C fallback */
#ifdef __cplusplus
class ExplicitClass {
public:
    explicit ExplicitClass(int x) : value(x) {}
private:
    int value;
};
#else
/* In C, use a struct with a special marker */
struct explicit_marker {
    int value;
};
#endif

/* For DW_AT_is_optional - optional parameters/variables */
/* Simulate with a union and flag */
struct optional_int {
    int has_value;
    int value;
};

/* For DW_AT_mutable - mutable data */
#ifdef __cplusplus
class MutableClass {
public:
    MutableClass() : counter(0) {}
    void increment() const { counter++; }  // mutable allows modification in const method
private:
    mutable int counter;
};
#else
/* In C, simulate with volatile to indicate changeable state */
struct mutable_sim {
    volatile int changeable;
};
#endif

/* For DW_AT_ordering - array ordering (column-major) */
#ifdef __GNUC__
int column_major_array[5][5] __attribute__((aligned(16)));
#else
int column_major_array[5][5];
#endif

/* For DW_AT_segment - segment-specific pointers */
/* Try to use segment attributes if available */
#ifdef __i386__
#ifdef __GNUC__
int * __attribute__((segment("fs"))) fs_segment_ptr;
#endif
#endif

/* For DW_AT_prototyped - function prototypes */
int prototyped_function(int a, char b, double c);
int (*func_ptr)(int, char, double) = &prototyped_function;

/* For DW_AT_small - packed/small types */
struct small_packed {
    unsigned int flag1 : 1;
    unsigned int flag2 : 1;
    unsigned int flag3 : 1;
    unsigned int flag4 : 1;
    unsigned int : 4;  /* padding */
} __attribute__((packed));

/* Additional complex types to stress DIE generation */
typedef struct {
    int x;
    int y;
    fixed_string name;
} point_t;

typedef enum {
    ORDER_ASCENDING,
    ORDER_DESCENDING,
    ORDER_UNSPECIFIED
} ordering_t;

/* Variable with various storage classes and qualifiers */
static int static_var = 100;
const int const_var = 200;
volatile int volatile_var = 300;
register int register_var asm("ebx");  /* Note: register selection is architecture-specific */

/* Thread-local storage */
#ifdef __GNUC__
__thread int thread_local_var = 400;
#endif

/* Restrict qualified pointer */
void use_restrict(int *restrict ptr1, int *restrict ptr2) {
    if (ptr1 != ptr2) {
        *ptr1 = *ptr2;
    }
}

/* Complex array with multiple dimensions */
int complex_array[3][4][5];

/* Function with many parameters to generate parameter DIEs */
int multi_param_func(int a, long b, float c, double d, 
                     char e, short f, int *g, const char *h) {
    return a + (int)b + (int)c + (int)d + e + f + *g + (int)*h;
}

/* Inline function to generate DW_AT_prototyped */
static inline int inline_func(int x, int y) {
    return x * y;
}

/* External linkage function definition */
int prototyped_function(int a, char b, double c) {
    return a + (int)b + (int)c;
}

/* Main function using all declared variables */
int main(int argc, char *argv[]) {
    /* Use string types */
    fixed_string local_string = "local_test";
    struct string_struct str_obj = {
        .dynamic_string = "dynamic",
        .fixed_array = "fixed_array_contents",
        .length = 21
    };
    
    /* Use picture data */
    struct picture_data pic = { .data = "PICTURE1234567890" };
    
    /* Use optional */
    struct optional_int opt = { .has_value = 1, .value = 999 };
    
    /* Use small packed struct */
    struct small_packed small = { .flag1 = 1, .flag2 = 0, .flag3 = 1, .flag4 = 0 };
    
    /* Use ordering array */
    for (int i = 0; i < 5; i++) {
        for (int j = 0; j < 5; j++) {
            column_major_array[i][j] = i * 10 + j;
        }
    }
    
    /* Use OpenMP if available */
    #ifdef _OPENMP
    #pragma omp parallel
    {
        omp_global_var = omp_get_thread_num();
    }
    #endif
    
    /* Use thread local */
    #ifdef __GNUC__
    thread_local_var = 500;
    #endif
    
    /* Use complex array */
    for (int i = 0; i < 3; i++) {
        for (int j = 0; j < 4; j++) {
            for (int k = 0; k < 5; k++) {
                complex_array[i][j][k] = i * 100 + j * 10 + k;
            }
        }
    }
    
    /* Call functions */
    int result = prototyped_function(10, 'A', 3.14);
    result += inline_func(5, 6);
    result += multi_param_func(1, 2, 3.0, 4.0, 'X', 5, &static_var, "test");
    
    /* Use restrict */
    int x = 10, y = 20;
    use_restrict(&x, &y);
    
    /* Return computed value to ensure variables are used */
    return result + local_string[0] + str_obj.length + pic.data[0] + 
           opt.value + small.flag1 + column_major_array[0][0] + 
           complex_array[0][0][0] + x + y;
}
