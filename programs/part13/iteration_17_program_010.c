/* Test program to trigger generation of specific DWARF attributes */
#include <stddef.h>

/* For DW_AT_string_length, DW_AT_string_length_bit_size, DW_AT_string_length_byte_size */
/* Fixed-length string array typedef */
typedef char fixed_string[32];
fixed_string global_string = "test_string";

/* For DW_AT_picture_string - COBOL/Fortran PICTURE data simulation */
#pragma pack(push, 1)
struct cobol_picture {
    char picture[10];
} cobol_var;
#pragma pack(pop)

/* For DW_AT_threads_scaled - OpenMP threadprivate variable */
#ifdef _OPENMP
int omp_thread_var;
#pragma omp threadprivate(omp_thread_var)
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

/* For DW_AT_is_optional - C++17 std::optional */
#ifdef __cplusplus
#include <optional>
std::optional<int> optional_var;
#endif

/* For DW_AT_mutable - C++ mutable member */
#ifdef __cplusplus
class MutableClass {
public:
    MutableClass() : normal(0), mutable_member(0) {}
    void modify() const { mutable_member = 42; }  // Can modify mutable in const method
private:
    int normal;
    mutable int mutable_member;
};
#endif

/* For DW_AT_ordering - Column-major array ordering */
#ifdef __GNUC__
int column_major_array[5][5] __attribute__((aligned(16)));
#endif

/* For DW_AT_segment - Segment-specific pointers */
#ifdef __i386__
/* x86 segment registers */
int __seg_fs *fs_seg_ptr;
int __seg_gs *gs_seg_ptr;
#endif

/* For DW_AT_prototyped - Function with prototype */
int prototyped_function(int a, char b);
int (*prototyped_ptr)(int, char) = &prototyped_function;

/* For DW_AT_small - Packed struct with bit-field */
struct SmallPacked {
    unsigned int flag:1;
    unsigned int value:7;
} __attribute__((packed));

/* Additional complex types to stress DIE generation */
typedef struct {
    fixed_string name;
    int id;
    struct SmallPacked flags;
} ComplexType;

/* Thread-local storage */
__thread int thread_local_var = 42;

/* Volatile and const qualified variables */
volatile int volatile_var = 100;
const int const_var = 200;

/* Restrict qualified pointer */
void use_restrict(int *restrict ptr1, int *restrict ptr2) {
    *ptr1 = 1;
    *ptr2 = 2;
}

/* Function with variable arguments */
#include <stdarg.h>
void variadic_function(int count, ...) {
    va_list args;
    va_start(args, count);
    for (int i = 0; i < count; i++) {
        int val = va_arg(args, int);
    }
    va_end(args);
}

/* Inline function */
static inline int inline_func(int x) {
    return x * 2;
}

/* Static and extern variables */
static int static_var = 300;
extern int extern_var;  // Defined elsewhere

/* Register variable (suggestion only) */
register int register_var asm("ebx");

/* Array of function pointers */
typedef int (*func_ptr_t)(int);
func_ptr_t func_array[5];

/* Nested struct with anonymous union */
struct Nested {
    int type;
    union {
        int int_val;
        float float_val;
        fixed_string str_val;
    } data;
};

/* Main function using various constructs */
int prototyped_function(int a, char b) {
    return a + (int)b;
}

int main() {
    /* Local instances of our types */
    fixed_string local_string = "local";
    struct cobol_picture local_cobol;
    
    struct SmallPacked small = {0, 42};
    
#ifdef __cplusplus
    ExplicitClass expl(10);
    MutableClass mut;
    mut.modify();
    optional_var = 42;
#endif
    
#ifdef _OPENMP
    #pragma omp parallel
    {
        omp_thread_var = omp_get_thread_num();
    }
#endif
    
    /* Use column-major array */
#ifdef __GNUC__
    for (int i = 0; i < 5; i++) {
        for (int j = 0; j < 5; j++) {
            column_major_array[i][j] = i * 5 + j;
        }
    }
#endif
    
    /* Use segment pointers */
#ifdef __i386__
    /* These would need proper initialization in real code */
#endif
    
    /* Use function pointer */
    int result = prototyped_ptr(10, 'A');
    
    /* Use thread-local */
    thread_local_var++;
    
    /* Use restrict */
    int x, y;
    use_restrict(&x, &y);
    
    /* Use variadic */
    variadic_function(3, 1, 2, 3);
    
    /* Use inline */
    int doubled = inline_func(42);
    
    /* Use complex type */
    ComplexType complex = {"test", 1, {0, 127}};
    
    /* Use nested struct */
    struct Nested nested = {1, {.int_val = 100}};
    
    return 0;
}

/* External variable definition */
int extern_var = 400;
