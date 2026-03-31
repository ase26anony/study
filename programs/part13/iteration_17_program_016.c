/* test_dwarf_attrs.c - Comprehensive test for various DWARF attributes */
/* Compile with: gcc -g -O0 -fopenmp -std=c++17 test_dwarf_attrs.c -o test_dwarf_attrs */

#include <stddef.h>
#include <stdint.h>

/* For DW_AT_string_length, DW_AT_string_length_bit_size, DW_AT_string_length_byte_size */
/* Fixed-length string type that might trigger string length attributes */
typedef char fixed_string[32];
fixed_string global_string = "test_string_for_dwarf";

/* Structure with string member that might have length attributes */
struct string_struct {
    char *data;
    size_t length;
};

/* For DW_AT_picture_string - COBOL/Fortran PICTURE data simulation */
/* Use GCC's __attribute__((picture)) if available */
#ifdef __GNUC__
struct picture_data {
    char digits[10];
} __attribute__((picture("9(5)V9(2)")));
#else
struct picture_data {
    char digits[10];
};
#endif

/* For DW_AT_threads_scaled - OpenMP thread-related variables */
#ifdef _OPENMP
int omp_global_var = 0;
#pragma omp threadprivate(omp_global_var)

/* Thread-local storage */
__thread int thread_local_var = 42;
#endif

/* For DW_AT_explicit - C++ explicit constructor (in C mode, we'll use C++ section) */
#ifdef __cplusplus
class ExplicitClass {
public:
    explicit ExplicitClass(int x) : value(x) {}
private:
    int value;
};
#endif

/* For DW_AT_is_optional - Simulate optional types */
/* In C, we can use a struct with a flag */
struct optional_int {
    int has_value;
    int value;
};

/* For DW_AT_mutable - C++ mutable member (in C mode, simulate) */
#ifdef __cplusplus
class MutableClass {
public:
    MutableClass() : counter(0) {}
    void increment() const { counter++; }  // Can modify mutable in const method
private:
    mutable int counter;
};
#endif

/* For DW_AT_ordering - Array ordering (column-major for Fortran) */
#ifdef __GNUC__
int column_major_array[5][5] __attribute__((column_major));
#else
int column_major_array[5][5];
#endif

/* For DW_AT_segment - Segment-specific pointers */
/* Use compiler-specific segment attributes */
#ifdef __i386__
int __seg_fs *fs_segment_ptr;
int __seg_gs *gs_segment_ptr;
#endif

/* For DW_AT_prototyped - Functions with full prototypes */
int prototyped_function(int a, char b, double c);
int (*func_ptr)(int, char, double) = &prototyped_function;

/* For DW_AT_small - Packed structures with small types */
struct small_packed {
    unsigned int flag : 1;
    unsigned int tiny : 3;
} __attribute__((packed));

/* Complex type compositions to stress DIE generation */
typedef int (*complex_func_ptr)(struct string_struct*, const volatile void* restrict);

/* Variables with different storage classes */
static int static_var = 100;
register int register_var asm ("r12") = 200;  /* May not work on all targets */
volatile int volatile_var = 300;
const int const_var = 400;
int *restrict restrict_ptr = NULL;

/* Multi-dimensional array with various qualifiers */
const volatile int cv_array[3][4][5];

/* Function returning pointer to array */
int (*function_returning_array_ptr(int x))[10] {
    static int array[10];
    return &array;
}

/* Nested structures */
struct outer {
    struct inner {
        int a;
        double b;
    } nested;
    union {
        int x;
        float y;
    } data;
};

/* Bitfields of various sizes */
struct bitfield_test {
    unsigned int a : 1;
    signed int b : 2;
    unsigned int c : 15;
    signed int d : 14;
};

/* Variable with alignment specification */
int aligned_var __attribute__((aligned(64)));

/* String literal pointer - might trigger string attributes */
const char *string_literal = "Hello, DWARF!";

/* Array of strings */
const char *string_array[] = {"one", "two", "three", NULL};

/* Function implementations */
int prototyped_function(int a, char b, double c) {
    return a + (int)b + (int)c;
}

/* Main test function */
int main(int argc, char *argv[]) {
    /* Local variables with various types */
    fixed_string local_string = "local_test";
    struct string_struct str = {local_string, sizeof(local_string)};
    
    struct picture_data pic = {{'1','2','3','4','5','.','6','7'}};
    
    struct optional_int opt = {1, 42};
    
    struct small_packed sp = {1, 5};
    
    int local_array[10][10];
    
    /* Initialize column-major array */
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
    
    /* Use all variables to prevent optimization */
    int sum = static_var + volatile_var + const_var;
    if (restrict_ptr) sum += *restrict_ptr;
    
    sum += prototyped_function(1, 'a', 3.14);
    sum += opt.value;
    sum += sp.flag + sp.tiny;
    
    for (int i = 0; i < 5; i++) {
        for (int j = 0; j < 5; j++) {
            sum += column_major_array[i][j];
        }
    }
    
    /* Use string-related variables */
    sum += (int)str.length;
    for (int i = 0; string_array[i]; i++) {
        sum += (int)string_array[i][0];
    }
    
    return sum > 0 ? 0 : 1;
}

/* Additional C++ specific code in separate compilation unit */
#ifdef __cplusplus
#include <optional>

/* C++ specific tests */
std::optional<int> cpp_optional_var = 42;

class TestAllAttributes {
public:
    explicit TestAllAttributes(int x) : value(x) {}
    
    void method() const {
        mutable_member++;  // Modify mutable in const method
    }
    
private:
    int value;
    mutable int mutable_member = 0;
};

/* Template class to generate complex debug info */
template<typename T>
class TemplateClass {
    T data;
public:
    explicit TemplateClass(const T& d) : data(d) {}
    T get() const { return data; }
};

void test_cpp_features() {
    TestAllAttributes obj(10);
    obj.method();
    
    TemplateClass<int> tc(42);
    int val = tc.get();
    
    cpp_optional_var = 100;
}
#endif
