/* test_dwarf_attrs.c - Comprehensive test for triggering specific DWARF attributes */
/* Compile with: gcc -g -O0 -dA -fopenmp -std=c++17 test_dwarf_attrs.c -o test_dwarf_attrs */

#include <stddef.h>
#include <stdint.h>

/* For DW_AT_string_length, DW_AT_string_length_bit_size, DW_AT_string_length_byte_size */
/* Fixed-length string type that might trigger string length attributes */
typedef char fixed_string[32];
fixed_string global_string = "Hello, DWARF!";

/* For DW_AT_picture_string - COBOL/Fortran PICTURE data simulation */
#ifdef __GNUC__
/* GCC extension for PICTURE strings (may not be available on all targets) */
struct cobol_picture {
    char data[20];
} __attribute__((packed));
#else
struct cobol_picture {
    char data[20];
};
#endif

/* For DW_AT_threads_scaled - OpenMP thread-related variables */
#ifdef _OPENMP
#include <omp.h>
int omp_thread_var = 0;
#pragma omp threadprivate(omp_thread_var)
#endif

/* For DW_AT_explicit - C++ explicit constructor */
#ifdef __cplusplus
class ExplicitClass {
public:
    explicit ExplicitClass(int x) : value(x) {}
    int getValue() const { return value; }
private:
    int value;
};
#endif

/* For DW_AT_is_optional - C++17 std::optional */
#ifdef __cplusplus
#include <optional>
std::optional<int> optional_var = 42;
#endif

/* For DW_AT_mutable - C++ mutable member */
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

/* For DW_AT_segment - Segment-specific pointers (x86-specific) */
#ifdef __x86_64__
/* Use segment register attributes if available */
#ifdef __SEG_FS
int __seg_fs *fs_seg_ptr = 0;
#endif
#ifdef __SEG_GS
int __seg_gs *gs_seg_ptr = 0;
#endif
#endif

/* For DW_AT_prototyped - Function with full prototype */
int prototyped_function(int a, char b, double c);
int (*func_ptr)(int, char, double) = &prototyped_function;

/* For DW_AT_small - Packed structure with bit-fields */
struct SmallPacked {
    unsigned int flag1 : 1;
    unsigned int flag2 : 1;
    unsigned int flag3 : 1;
    unsigned int flag4 : 1;
} __attribute__((packed));

/* For DW_AT_location - Variables with complex locations */
static int static_var = 100;
volatile int volatile_var = 200;
register int register_var asm("r12") = 300;

/* For DW_AT_lower_bound - Array with specified bounds */
int bounded_array[10] = {0};

/* Complex type compositions to stress DIE generation */
typedef int (*complex_func_ptr)(int (*)(char), double[3][3]);
typedef struct {
    int x;
    double y;
    char z[10];
} complex_struct;

/* Function declarations with various attributes */
#ifdef __GNUC__
int __attribute__((noinline)) noinline_func(int x) {
    return x * 2;
}

int __attribute__((always_inline)) inline_func(int x) {
    return x + 1;
}
#endif

/* Main test function */
int prototyped_function(int a, char b, double c) {
    /* Local variables with different storage classes */
    auto int auto_var = a;
    static int local_static = 0;
    
    /* Thread-local variable */
    #ifdef __STDC_NO_THREADS__
    _Thread_local int thread_local_var = 0;
    #endif
    
    /* Register variable */
    register int local_register asm("r13") = b;
    
    /* Complex array */
    int multi_array[2][3][4] = {0};
    
    /* Pointer to array */
    int (*array_ptr)[3][4] = &multi_array[0];
    
    /* Reference to volatile */
    volatile int *volatile_ptr = &volatile_var;
    
    /* Restrict pointer */
    int *restrict restrict_ptr = &auto_var;
    
    /* Compute result */
    int result = auto_var + local_register + static_var;
    
    /* Use OpenMP if available */
    #ifdef _OPENMP
    #pragma omp parallel
    {
        int tid = omp_get_thread_num();
        omp_thread_var = tid;
    }
    #endif
    
    /* Use packed structure */
    struct SmallPacked packed = {1, 0, 1, 0};
    
    /* Use column-major array */
    for (int i = 0; i < 5; i++) {
        for (int j = 0; j < 5; j++) {
            column_major_array[i][j] = i * 5 + j;
        }
    }
    
    return result;
}

/* Additional test functions */
void test_string_attributes(void) {
    /* String operations that might trigger string length attributes */
    fixed_string local_str = "Local string";
    char *str_ptr = local_str;
    
    /* Array of strings */
    char string_array[3][20] = {"First", "Second", "Third"};
    
    /* Pointer to string array */
    char (*str_array_ptr)[20] = string_array;
}

void test_complex_types(void) {
    /* Nested structures */
    struct Outer {
        struct Inner {
            int a;
            double b;
        } inner;
        char name[20];
    } outer = {{42, 3.14}, "Test"};
    
    /* Union with bit-fields */
    union BitUnion {
        struct {
            unsigned int low : 8;
            unsigned int high : 8;
        } bits;
        unsigned short value;
    } bit_union = {.bits = {0xFF, 0xAA}};
    
    /* Function pointer array */
    int (*func_array[5])(int, char, double);
    
    /* Variably modified type */
    int n = 10;
    int vla[n][n];
    
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < n; j++) {
            vla[i][j] = i * j;
        }
    }
}

/* Main function */
int main(int argc, char *argv[]) {
    int result = 0;
    
    /* Test all features */
    result = prototyped_function(10, 'A', 3.14159);
    
    test_string_attributes();
    test_complex_types();
    
    /* C++ specific tests if compiled as C++ */
    #ifdef __cplusplus
    ExplicitClass expl(42);
    MutableClass mut;
    mut.increment();
    
    if (optional_var.has_value()) {
        result += optional_var.value();
    }
    #endif
    
    /* Use packed structure */
    struct SmallPacked packed = {1, 1, 0, 0};
    
    /* Use bounded array */
    for (int i = 0; i < 10; i++) {
        bounded_array[i] = i * i;
    }
    
    /* Use global string */
    global_string[0] = 'M';
    
    return result;
}

/* Additional global variables for more DIEs */
extern int external_var;
const int const_global = 1000;
volatile int volatile_global = 2000;

/* Array of function pointers */
typedef int (*math_func_t)(int, int);
math_func_t math_funcs[] = {
    NULL, NULL, NULL
};

/* Anonymous struct/union */
struct {
    int type;
    union {
        int int_val;
        double double_val;
        char *str_val;
    } data;
} anonymous_var = {1, {.int_val = 42}};
