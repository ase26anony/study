/* test_dwarf_attrs.c - Comprehensive test for triggering specific DWARF attributes */
/* Compile with: gcc -g -dA -O0 -fopenmp -std=c++17 test_dwarf_attrs.c -o test_dwarf_attrs */

#include <stddef.h>
#include <stdint.h>

/* For C++ features */
#ifdef __cplusplus
#include <optional>
#include <string>
#endif

/* ===== DW_AT_string_length, DW_AT_string_length_bit_size, DW_AT_string_length_byte_size ===== */
/* Fixed-length string types that might trigger string length attributes */
typedef char fixed_string_32[32];
typedef struct {
    char data[64];
    size_t length;
} string_with_length;

/* Fortran-style CHARACTER declaration simulation */
#ifdef __GNUC__
typedef struct {
    char *data;
    int length;
} CHARACTER __attribute__((fortran_character));
#endif

/* ===== DW_AT_picture_string ===== */
/* COBOL/Fortran PICTURE data simulation */
#ifdef __GNUC__
struct cobol_picture {
    char picture[20];
} __attribute__((packed));

/* Attempt to use picture attribute if supported */
#if defined(__has_attribute) && __has_attribute(picture)
typedef char picture_string __attribute__((picture("9(5)V9(2)")));
#else
typedef char picture_string[10];
#endif
#endif

/* ===== DW_AT_threads_scaled ===== */
/* OpenMP thread-related variables */
#ifdef _OPENMP
int omp_global_var = 42;
#pragma omp threadprivate(omp_global_var)

int omp_thread_scaled[10];
#pragma omp declare target(omp_thread_scaled)
#endif

/* Thread-local storage */
__thread int thread_local_var = 0;

/* ===== DW_AT_explicit ===== */
#ifdef __cplusplus
class ExplicitClass {
public:
    explicit ExplicitClass(int x) : value(x) {}
    explicit operator bool() const { return value != 0; }
private:
    int value;
};

class WithExplicit {
public:
    explicit WithExplicit(int) {}
    explicit WithExplicit(double) {}
};
#endif

/* ===== DW_AT_is_optional ===== */
#ifdef __cplusplus
std::optional<int> global_optional;
std::optional<double> another_optional = 3.14;
#endif

/* Struct with optional-like semantics */
struct maybe_present {
    int is_present;
    union {
        int value;
        char dummy;
    };
};

/* ===== DW_AT_mutable ===== */
#ifdef __cplusplus
class ClassWithMutable {
public:
    ClassWithMutable() : normal_member(0) {}
    int get_value() const { 
        mutable_counter++;  // Can modify in const function
        return normal_member;
    }
private:
    int normal_member;
    mutable int mutable_counter;
};

struct MutableStruct {
    mutable int can_change;
    const int cannot_change;
};
#endif

/* ===== DW_AT_ordering ===== */
/* Column-major array ordering simulation */
#ifdef __GNUC__
int column_major_array[5][5] __attribute__((column_major));

/* Fortran-style array */
typedef int fortran_matrix[10][10] __attribute__((fortran_array));
#endif

/* Multi-dimensional arrays with different storage orders */
int row_major[3][4][5];  /* Default C row-major */
int nested_array[2][3] = {{1,2,3}, {4,5,6}};

/* ===== DW_AT_segment ===== */
/* Segment-specific pointers (x86 memory models) */
#ifdef __i386__
#ifdef __GNUC__
int * __seg_fs fs_segment_ptr;
int * __seg_gs gs_segment_ptr;
#endif

/* Far pointer simulation */
typedef int __far * far_ptr_t;
#endif

/* Different storage classes */
static int static_var = 1;
extern int extern_var;
register int register_var asm("ebx");  /* May not work on all targets */

/* ===== DW_AT_prototyped ===== */
/* Fully prototyped functions */
int prototyped_func(int a, char b, double c);
int (*func_ptr)(int, char, double) = &prototyped_func;

/* Function with ellipsis */
int variadic_func(const char *fmt, ...);

/* Function returning complex type */
struct complex_return {
    int a;
    double b;
    char c[10];
};

struct complex_return (*complex_func_ptr)(int, float) = NULL;

/* ===== DW_AT_small ===== */
/* Packed structures and bit-fields */
struct packed_small {
    unsigned int flag1 : 1;
    unsigned int flag2 : 1;
    unsigned int flag3 : 1;
    unsigned int : 5;  /* Padding */
    unsigned char tiny : 2;
} __attribute__((packed));

/* Very small packed struct */
struct tiny_struct {
    unsigned char a : 3;
    unsigned char b : 3;
    unsigned char c : 2;
} __attribute__((packed, aligned(1)));

/* ===== Additional type qualifiers and storage classes ===== */
const volatile int const_volatile_var = 42;
int * restrict restrict_ptr = NULL;

/* Complex type composition */
typedef int (*complex_func_type)(const char * restrict, ...);
complex_func_type complex_func_var = NULL;

/* Array of function pointers */
int (*func_array[5])(int, int);

/* ===== Main function with local variables ===== */
int prototyped_func(int a, char b, double c) {
    /* Local variables with various attributes */
    static int local_static = 0;
    
#ifdef __cplusplus
    std::optional<int> local_optional = a;
    ClassWithMutable mutable_obj;
    mutable_obj.get_value();  // Use mutable member
#endif
    
    /* Fixed string local */
    fixed_string_32 local_string = "Hello, DWARF!";
    
    /* Thread-local in function */
    static __thread int func_thread_local = 0;
    func_thread_local++;
    
    /* Register variable attempt */
    register int local_register asm("eax") = a;
    
    return a + (int)b + (int)c + local_register;
}

#ifdef __cplusplus
/* C++ specific test cases */
void test_cpp_features() {
    ExplicitClass expl(42);
    WithExplicit wex(3.14);
    
    global_optional = 100;
    another_optional.reset();
    
    ClassWithMutable mut;
    mut.get_value();
    
    std::string cpp_string = "C++ string";
    cpp_string.length();  // Use string
}
#endif

/* Variadic function implementation */
int variadic_func(const char *fmt, ...) {
    (void)fmt;
    return 0;
}

/* Main test driver */
int main(int argc, char *argv[]) {
    /* Initialize and use various variables */
    fixed_string_32 my_string = "Test string";
    
#ifdef __cplusplus
    test_cpp_features();
#endif
    
    /* Use OpenMP if available */
#ifdef _OPENMP
    #pragma omp parallel
    {
        omp_global_var = omp_get_thread_num();
        for (int i = 0; i < 10; i++) {
            omp_thread_scaled[i] = i * omp_get_thread_num();
        }
    }
#endif
    
    /* Use prototyped function */
    int result = prototyped_func(10, 'A', 3.14);
    
    /* Use packed struct */
    struct packed_small small = {1, 0, 1, 2};
    
    /* Use column-major array if available */
#ifdef __GNUC__
    column_major_array[0][0] = 1;
#endif
    
    return result + small.flag1 + argc;
}

/* External variable definition */
int extern_var = 100;

/* Complex function return type function */
struct complex_return complex_return_func(int x, float y) {
    struct complex_return cr = {x, (double)y, "test"};
    return cr;
}
