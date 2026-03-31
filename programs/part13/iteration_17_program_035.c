/* test_dwarf_attrs.c - Comprehensive test for various DWARF attributes */
/* Compile with: gcc -g -O0 -dA -fopenmp -std=c++17 test_dwarf_attrs.c -o test_dwarf_attrs */

#include <stddef.h>
#include <stdint.h>

/* For DW_AT_string_length, DW_AT_string_length_bit_size, DW_AT_string_length_byte_size */
/* Fixed-length string types that might generate string length attributes */
typedef char fixed_string_32[32];
typedef struct {
    char data[64];
    size_t length;
} string_with_length;

/* For DW_AT_picture_string - COBOL/Fortran PICTURE data */
/* Use GCC's attribute if available */
#ifdef __GNUC__
struct picture_data {
    char value[20];
} __attribute__((packed));
#else
struct picture_data {
    char value[20];
};
#endif

/* For DW_AT_threads_scaled - OpenMP thread-related variables */
#ifdef _OPENMP
int omp_global_var = 42;
#pragma omp threadprivate(omp_global_var)
#endif

/* For DW_AT_explicit - C++ explicit constructor (in C++ section) */
/* For DW_AT_is_optional - std::optional (in C++ section) */
/* For DW_AT_mutable - mutable member (in C++ section) */

/* For DW_AT_ordering - array ordering (Fortran column-major) */
#ifdef __GNUC__
int column_major_array[5][5] __attribute__((column_major));
#else
int column_major_array[5][5];
#endif

/* For DW_AT_segment - segment-specific pointers */
#ifdef __i386__
int __seg_fs *fs_seg_ptr;
int __seg_gs *gs_seg_ptr;
#endif

/* For DW_AT_prototyped - function with prototype */
int prototyped_function(int a, char b, double c);
int (*func_ptr)(int, char, double) = &prototyped_function;

/* For DW_AT_small - packed small types */
struct small_packed {
    unsigned int flag:1;
    unsigned int value:7;
} __attribute__((packed));

/* For DW_AT_location - variables with specific locations */
register int reg_var asm("ebx");

/* For DW_AT_lower_bound - array with non-zero lower bound */
#ifdef __GNUC__
int bounded_array[10] __attribute__((aligned(16)));
#else
int bounded_array[10];
#endif

/* Complex type compositions to stress DIE generation */
typedef int (*complex_func_ptr)(int (*)(char), double[10]);
typedef struct {
    volatile int *const volatile restrict ptr;
    const struct small_packed packed;
} complex_type;

/* Storage class variations */
static int static_var = 100;
thread_local int tls_var = 200;
const int const_var = 300;
volatile int volatile_var = 400;
restrict int *restrict_ptr;

/* Function definitions */
int prototyped_function(int a, char b, double c) {
    return a + (int)b + (int)c;
}

/* Main function with various scopes */
int main(int argc, char *argv[]) {
    /* Local variables with different types */
    fixed_string_32 str = "Hello, DWARF!";
    string_with_length str_len = {.data = "Test", .length = 4};
    struct picture_data pic = {.value = "99999V99"};
    struct small_packed small = {.flag = 1, .value = 42};
    
    /* Array with potential ordering */
    int local_array[3][3] = {{1,2,3},{4,5,6},{7,8,9}};
    
    /* Pointer with segment attribute */
#ifdef __i386__
    int local_var = 50;
    fs_seg_ptr = &local_var;
#endif
    
    /* OpenMP region for threads_scaled */
#ifdef _OPENMP
    #pragma omp parallel
    {
        omp_global_var = omp_get_thread_num();
    }
#endif
    
    /* Complex type usage */
    complex_func_ptr cfp = NULL;
    complex_type ct = {.ptr = &volatile_var, .packed = small};
    
    /* Use all variables to prevent optimization */
    int sum = static_var + tls_var + const_var + volatile_var;
    if (restrict_ptr) sum += *restrict_ptr;
    sum += str[0] + str_len.length + pic.value[0] + small.value;
    sum += column_major_array[0][0] + bounded_array[0];
    sum += local_array[0][0];
    
    return sum;
}

/* Additional global variables */
extern int external_var;
int *global_ptr = &static_var;

/* Inline assembly to force specific code generation */
void asm_helper(void) {
    __asm__ volatile (
        "nop\n\t"
        "nop\n\t"
        : : : "memory"
    );
}

/* C++ specific section */
#ifdef __cplusplus
#include <optional>
#include <string>

/* For DW_AT_explicit */
class ExplicitClass {
public:
    explicit ExplicitClass(int x) : value(x) {}
    ExplicitClass(double x) : value((int)x) {}  // non-explicit
private:
    int value;
};

/* For DW_AT_is_optional */
std::optional<int> optional_var;
std::optional<std::string> optional_string;

/* For DW_AT_mutable */
class MutableClass {
public:
    MutableClass() : x(0) {}
    int get() const { 
        ++mutable_counter;  // Can modify in const function
        return x; 
    }
private:
    int x;
    mutable int mutable_counter = 0;
};

/* Template to generate more complex DIEs */
template<typename T>
class TemplateClass {
    T value;
public:
    explicit TemplateClass(T v) : value(v) {}
    T get() const { return value; }
};

/* Use the C++ classes */
void cpp_test() {
    ExplicitClass e1(42);      // explicit constructor
    ExplicitClass e2 = 3.14;   // implicit conversion
    
    optional_var = 42;
    optional_string = "test";
    
    MutableClass m;
    int val = m.get();
    
    TemplateClass<int> tc(100);
    val += tc.get();
}
#endif
