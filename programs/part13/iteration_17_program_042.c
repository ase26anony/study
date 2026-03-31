/* test_dwarf_attrs.c - Comprehensive test for triggering specific DWARF attributes */
/* Compile with: gcc -g -O0 -dA -fopenmp -std=c++17 test_dwarf_attrs.c -o test_dwarf_attrs */

#include <stddef.h>
#include <stdint.h>

/* For C++ features in C mode, we'll use compiler extensions */
#ifdef __cplusplus
#include <optional>
#endif

/* ===== DW_AT_string_length, DW_AT_string_length_bit_size, DW_AT_string_length_byte_size ===== */
/* Fixed-length string types that might trigger string length attributes */
typedef char fixed_string_32[32];
typedef struct {
    char data[64];
    size_t length;  /* Simulated string length field */
} string_with_length;

/* Fortran-style CHARACTER declaration using GCC attribute */
typedef struct {
    char str[80];
} __attribute__((fortran_character)) fortran_string;

/* ===== DW_AT_picture_string ===== */
/* COBOL/Fortran PICTURE data simulation */
#pragma pack(push, 1)
struct cobol_picture {
    /* Try to simulate PICTURE data - GCC may recognize this pattern */
    char digits[15];
    char decimal_point;
    char fraction[2];
} __attribute__((packed));
#pragma pack(pop)

/* Alternative: Use GCC's picture attribute if available */
#ifdef __GNUC__
/* Check if picture attribute is supported */
#if __GNUC__ > 4 || (__GNUC__ == 4 && __GNUC_MINOR__ >= 8)
typedef char picture_string __attribute__((picture("9(5)V9(2)")));
#else
typedef char picture_string[10];  /* Fallback */
#endif
#else
typedef char picture_string[10];
#endif

/* ===== DW_AT_threads_scaled ===== */
/* OpenMP thread-related variables */
#ifdef _OPENMP
int omp_global_var = 42;
#pragma omp threadprivate(omp_global_var)

/* Variable in OpenMP target region */
#pragma omp declare target
int target_var = 100;
#pragma omp end declare target
#endif

/* Thread-local storage */
__thread int thread_local_var = 0;

/* ===== DW_AT_explicit ===== */
/* C++ explicit constructor - will be in C++ section */

/* ===== DW_AT_is_optional ===== */
/* Optional parameter simulation */
struct optional_param {
    int value;
    unsigned char is_present;
};

/* Function with optional-like parameter */
void func_with_optional(struct optional_param *opt) {
    if (opt && opt->is_present) {
        /* Use opt->value */
    }
}

/* ===== DW_AT_mutable ===== */
/* C++ mutable member - will be in C++ section */

/* ===== DW_AT_ordering ===== */
/* Array ordering attributes */
#ifdef __GNUC__
/* Column-major array (Fortran style) */
int column_major_array[5][5] __attribute__((column_major));

/* Row-major array (C style) with explicit attribute */
int row_major_array[5][5] __attribute__((row_major));
#else
int column_major_array[5][5];
int row_major_array[5][5];
#endif

/* ===== DW_AT_segment ===== */
/* Segment-specific pointers */
#ifdef __i386__
/* x86 segment registers */
int __seg_fs *fs_pointer;
int __seg_gs *gs_pointer;
#endif

/* Near/far pointer simulation */
#ifdef __MSDOS__
int __far *far_pointer;
int __near *near_pointer;
#endif

/* Generic segment attribute attempt */
typedef struct {
    void *address;
    unsigned short segment;
} segmented_pointer;

/* ===== DW_AT_prototyped ===== */
/* Fully prototyped functions */
int prototyped_func(int a, char b, double c);
int (*func_ptr)(int, char, double) = &prototyped_func;

/* Another prototyped function with complex signature */
void complex_proto(struct cobol_picture *pic, fixed_string_32 str, int count);

/* ===== DW_AT_small ===== */
/* Packed structures and bit-fields */
struct small_packed {
    unsigned int flag1 : 1;
    unsigned int flag2 : 1;
    unsigned int flag3 : 1;
    unsigned int flag4 : 1;
    unsigned int value : 4;
} __attribute__((packed));

/* Very small bit-field structure */
struct tiny_bits {
    unsigned int a : 3;
    unsigned int b : 2;
    unsigned int c : 1;
} __attribute__((packed, aligned(1)));

/* ===== Complex type compositions ===== */
/* Mix of storage classes and qualifiers */
static const fixed_string_32 const_string = "Hello, World!";
volatile int volatile_counter = 0;
register int register_var asm("ebx");  /* Hint for register storage */

/* Restrict qualified pointer */
void copy_string(char *restrict dest, const char *restrict src, size_t n) {
    for (size_t i = 0; i < n; i++) {
        dest[i] = src[i];
    }
}

/* Function returning pointer to array */
int (*func_returning_array_ptr(void))[5] {
    static int array[5][5];
    return array;
}

/* ===== Variables in different scopes ===== */
/* File scope variables */
fixed_string_32 global_string = "Global";
static int static_global = 100;

/* Function with local variables */
void test_function(void) {
    /* Local variables with various attributes */
    auto int auto_var = 42;
    static int static_local = 0;
    const int const_local = 100;
    volatile int volatile_local = 0;
    
    /* Array with different ordering */
    int local_array[3][4];
    
    /* String variable */
    fixed_string_32 local_string;
    
    /* Packed structure */
    struct small_packed packed_local = {1, 0, 1, 0, 7};
    
    /* Use variables to avoid warnings */
    (void)auto_var;
    (void)static_local;
    (void)const_local;
    (void)volatile_local;
    (void)local_array;
    (void)local_string;
    (void)packed_local;
}

/* ===== C++ Specific Section ===== */
#ifdef __cplusplus

/* DW_AT_explicit - C++ class with explicit constructor */
class ExplicitClass {
public:
    explicit ExplicitClass(int value) : data(value) {}
    ExplicitClass(double value) : data(static_cast<int>(value)) {}  // Non-explicit
private:
    int data;
};

/* DW_AT_is_optional - C++17 std::optional */
std::optional<int> optional_value;
std::optional<double> optional_double = 3.14;

/* DW_AT_mutable - C++ class with mutable member */
class MutableClass {
public:
    MutableClass() : counter(0) {}
    void increment() const { counter++; }  // Can modify mutable member in const function
private:
    mutable int counter;
    const int const_member = 42;
};

/* Template with optional-like behavior */
template<typename T>
class OptionalTemplate {
    T value;
    bool has_value;
public:
    explicit OptionalTemplate(const T& val) : value(val), has_value(true) {}
    OptionalTemplate() : has_value(false) {}
};

#endif /* __cplusplus */

/* ===== Main function ===== */
int main(int argc, char *argv[]) {
    /* Initialize and use various variables */
    fixed_string_32 my_string = "Test String";
    struct cobol_picture picture_data = {"123456789012345", '.', "12"};
    struct small_packed packed_data = {1, 0, 1, 1, 5};
    
    /* Use OpenMP if available */
#ifdef _OPENMP
    #pragma omp parallel
    {
        omp_global_var = omp_get_thread_num();
        thread_local_var++;
    }
#endif
    
    /* Call prototyped function */
    int result = prototyped_func(10, 'A', 3.14);
    
    /* Use function pointer */
    result = func_ptr(20, 'B', 6.28);
    
    /* Test string copy with restrict */
    char dest[32];
    copy_string(dest, my_string, sizeof(my_string));
    
    /* Call test function */
    test_function();
    
#ifdef __cplusplus
    /* Use C++ features */
    ExplicitClass expl(42);
    ExplicitClass impl = 3.14;  // Should use non-explicit constructor
    
    MutableClass mut;
    mut.increment();
    
    optional_value = 100;
    if (optional_value.has_value()) {
        result = optional_value.value();
    }
#endif
    
    return 0;
}

/* Function implementations */
int prototyped_func(int a, char b, double c) {
    return a + (int)b + (int)c;
}

void complex_proto(struct cobol_picture *pic, fixed_string_32 str, int count) {
    /* Use parameters */
    (void)pic;
    (void)str;
    (void)count;
}
