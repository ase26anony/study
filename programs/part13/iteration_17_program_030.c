/* test_dwarf_attrs.c - Comprehensive test for triggering specific DWARF attributes */
/* Compile with: gcc -g -dA -O0 -fopenmp -std=c++17 test_dwarf_attrs.c -o test_dwarf_attrs */

#include <stddef.h>
#include <stdint.h>

/* For DW_AT_string_length, DW_AT_string_length_bit_size, DW_AT_string_length_byte_size */
/* Fixed-length string type (simulating Fortran CHARACTER) */
typedef char fixed_string[32];
fixed_string global_string = "Hello, DWARF!";

/* Structure with string length information */
struct string_struct {
    char *data;
    size_t length;
    int capacity;
};

/* For DW_AT_picture_string - COBOL-style picture data */
/* Use GCC attribute if available */
#ifdef __GNUC__
struct picture_data {
    char digits[10];
} __attribute__((packed));
#else
#pragma pack(push, 1)
struct picture_data {
    char digits[10];
};
#pragma pack(pop)
#endif

/* For DW_AT_threads_scaled - OpenMP thread-related variables */
#ifdef _OPENMP
#include <omp.h>
int omp_global_var = 42;
#pragma omp threadprivate(omp_global_var)

/* Thread-local storage */
__thread int thread_local_var = 100;

/* OpenMP target region variable */
int target_var = 200;
#endif

/* For DW_AT_explicit - C++ explicit constructor */
#ifdef __cplusplus
class ExplicitClass {
public:
    explicit ExplicitClass(int x) : value(x) {}
    int get_value() const { return value; }
private:
    int value;
};
#endif

/* For DW_AT_is_optional - Optional type simulation */
/* In C, we can simulate with a struct containing a flag */
struct optional_int {
    int value;
    int is_present;
};

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

/* For DW_AT_ordering - Array ordering (column-major simulation) */
/* Use attribute for column-major if supported */
#ifdef __GNUC__
int column_major_array[5][5] __attribute__((aligned(16)));
#else
int column_major_array[5][5];
#endif

/* For DW_AT_segment - Segment-specific pointers */
/* Use compiler-specific segment attributes if available */
#ifdef __i386__
#ifdef __GNUC__
int __attribute__((section(".data.fs"))) fs_data = 123;
int * __seg_fs fs_ptr = (int * __seg_fs)&fs_data;
#endif
#endif

/* For DW_AT_prototyped - Function with prototype */
int prototyped_function(int a, char b, double c);
int (*func_ptr)(int, char, double) = &prototyped_function;

/* For DW_AT_small - Packed small types */
struct small_struct {
    unsigned int flag1 : 1;
    unsigned int flag2 : 1;
    unsigned int flag3 : 1;
    unsigned int flag4 : 1;
} __attribute__((packed));

/* Complex type compositions to stress DIE generation */
typedef int (*complex_func_t)(int, struct string_struct*, double);

struct nested_struct {
    struct {
        int inner_a;
        char inner_b;
    } inner;
    union {
        int union_x;
        float union_y;
    } data;
    volatile int volatile_member;
    const char *const_string;
};

/* Storage class variations */
static int static_var = 999;
register int register_var asm("ebx");  /* Note: May not work on all targets */

/* Function definitions */
int prototyped_function(int a, char b, double c) {
    return a + (int)b + (int)c;
}

/* Main test function */
int main() {
    /* Local variables with various attributes */
    const int local_const = 42;
    volatile int local_volatile = 100;
    restrict int *restrict_ptr = &local_volatile;
    
    /* Use the fixed string */
    fixed_string local_string = "Local string";
    
    /* Use optional type */
    struct optional_int opt = { .value = 42, .is_present = 1 };
    
    /* Use small packed struct */
    struct small_struct small = { .flag1 = 1, .flag2 = 0, .flag3 = 1, .flag4 = 0 };
    
    /* Array with different storage */
    static int static_array[10];
    int auto_array[10];
    
    /* Initialize arrays */
    for (int i = 0; i < 10; i++) {
        static_array[i] = i * 2;
        auto_array[i] = i * 3;
    }
    
    /* Use column-major array */
    for (int i = 0; i < 5; i++) {
        for (int j = 0; j < 5; j++) {
            column_major_array[i][j] = i * 10 + j;
        }
    }
    
#ifdef _OPENMP
    /* OpenMP section for thread-scaled attributes */
    #pragma omp parallel
    {
        int thread_id = omp_get_thread_num();
        omp_global_var = thread_id;
        thread_local_var = thread_id * 10;
    }
    
    #pragma omp target map(tofrom: target_var)
    {
        target_var *= 2;
    }
#endif
    
#ifdef __cplusplus
    /* C++ specific tests */
    ExplicitClass expl_obj(42);
    MutableClass mut_obj;
    mut_obj.increment();
#endif
    
    /* Function pointer usage */
    int result = func_ptr(10, 'A', 3.14);
    
    /* Complex nested struct */
    struct nested_struct nested = {
        .inner = { .inner_a = 1, .inner_b = 'X' },
        .data = { .union_x = 100 },
        .volatile_member = 999,
        .const_string = "Constant string"
    };
    
    /* Pointer to array */
    int (*array_ptr)[5][5] = &column_major_array;
    
    /* Function returning struct */
    struct string_struct get_string_info(void) {
        struct string_struct info = {
            .data = "Dynamic",
            .length = 7,
            .capacity = 32
        };
        return info;
    }
    
    struct string_struct str_info = get_string_info();
    
    return 0;
}

/* Additional functions with varying prototypes */
static int static_func(int x) { return x * 2; }
inline int inline_func(int x) { return x + 1; }
__attribute__((noinline)) int noinline_func(int x) { return x - 1; }

/* Variable with asm label */
int asm_var __asm__("custom_asm_name") = 1234;

/* Weak symbol */
__attribute__((weak)) int weak_var = 0;

/* Aligned variable */
int aligned_var __attribute__((aligned(64))) = 456;

/* Cleanup attribute */
void cleanup_handler(int *p) {
    *p = 0;
}

int auto_cleanup_var __attribute__((cleanup(cleanup_handler))) = 789;

/* Transparent union */
union transparent_union {
    int *int_ptr;
    void *void_ptr;
} __attribute__((transparent_union));

/* Mode attribute for specific size */
typedef int int32 __attribute__((mode(SI)));
int32 mode_var = 32;

/* Vector type */
typedef int v4si __attribute__((vector_size(16)));
v4si vector_var = {1, 2, 3, 4};
