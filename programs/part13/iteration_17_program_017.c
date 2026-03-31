/* test_dwarf_attrs.c - Comprehensive test for triggering specific DWARF attributes */
/* Compile with: gcc -g -O0 -dA -fopenmp -std=c++17 test_dwarf_attrs.c -o test_dwarf_attrs */

#include <stddef.h>
#include <stdint.h>

/* For C++ features */
#ifdef __cplusplus
#include <optional>
#endif

/* ===== DW_AT_string_length, DW_AT_string_length_bit_size, DW_AT_string_length_byte_size ===== */
/* Fixed-length string type that might trigger string length attributes */
typedef char fixed_string[32];
fixed_string global_string = "Test string for length attributes";

/* String structure that might trigger length attributes */
struct string_struct {
    char *data;
    size_t length;
    size_t capacity;
};

/* ===== DW_AT_picture_string ===== */
/* Attempt to trigger picture string attribute (COBOL/Fortran style) */
#ifdef __GNUC__
/* GCC extension for picture strings (if supported) */
typedef struct {
    char data[20];
} __attribute__((picture("9(5)V9(2)"))) picture_string_t;
picture_string_t cobol_var;
#endif

/* Alternative approach using packed struct */
#pragma pack(push, 1)
struct cobol_picture {
    char digits[7];
    char decimal;
    char fraction[2];
};
#pragma pack(pop)
struct cobol_picture picture_data;

/* ===== DW_AT_threads_scaled ===== */
/* OpenMP thread-related variables */
#ifdef _OPENMP
#include <omp.h>
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
#ifdef __cplusplus
class ExplicitTest {
public:
    explicit ExplicitTest(int x) : value(x) {}
    explicit operator bool() const { return value != 0; }
private:
    int value;
};
#endif

/* ===== DW_AT_is_optional ===== */
#ifdef __cplusplus
std::optional<int> optional_var;
std::optional<double> optional_double = 3.14;
#endif

/* Function with optional-like parameter */
void func_with_optional_param(int required, int *optional_ptr) {
    if (optional_ptr) {
        *optional_ptr = required * 2;
    }
}

/* ===== DW_AT_mutable ===== */
#ifdef __cplusplus
class MutableTest {
public:
    MutableTest() : counter(0) {}
    int get_count() const {
        // mutable member can be modified in const function
        ++mutable_counter;
        return counter;
    }
private:
    int counter;
    mutable int mutable_counter;  /* Should trigger DW_AT_mutable */
};
#endif

/* ===== DW_AT_ordering ===== */
/* Column-major array (Fortran style) */
#ifdef __GNUC__
int column_major_array[5][5] __attribute__((column_major));
#endif

/* Multi-dimensional array with different storage orders */
int multi_array[3][4][5];

/* ===== DW_AT_segment ===== */
/* Segment-specific pointers (x86 memory models) */
#ifdef __x86_64__
/* FS/GS segment register pointers */
int * __seg_fs *fs_indirect_ptr = NULL;
int * __seg_gs *gs_indirect_ptr = NULL;
#endif

/* Near/far pointer simulation */
#ifdef __i386__
/* 32-bit far pointers */
int __far *far_ptr = NULL;
int __near *near_ptr = NULL;
#endif

/* ===== DW_AT_prototyped ===== */
/* Fully prototyped function */
int prototyped_function(int a, char b, double c);
int (*func_ptr)(int, char, double) = &prototyped_function;

int prototyped_function(int a, char b, double c) {
    return a + (int)b + (int)c;
}

/* Function pointer with prototype */
typedef int (*proto_func_t)(int, int);
proto_func_t proto_func_array[3];

/* ===== DW_AT_small ===== */
/* Packed structure with bit-fields */
struct SmallPacked {
    unsigned int flag1 : 1;
    unsigned int flag2 : 1;
    unsigned int value : 14;
    unsigned char tiny : 4;
} __attribute__((packed));

struct SmallPacked small_instance = {1, 0, 42, 3};

/* Tiny structure */
struct TinyStruct {
    signed char a : 3;
    signed char b : 3;
    signed char c : 2;
};

/* ===== Complex type compositions ===== */
/* Mix of storage classes and qualifiers */
static const volatile int static_const_volatile = 100;
register int register_var asm("ebx");
_Thread_local int thread_specific = 50;

/* Restrict pointers */
void use_restrict(int *restrict ptr1, int *restrict ptr2) {
    *ptr1 += *ptr2;
}

/* Complex pointer types */
int (*(*complex_func_ptr)(void))[10];
int *(*(*nested_ptr_array[5]))(float, double);

/* ===== Main test function ===== */
int main() {
    /* String length attributes test */
    fixed_string local_string = "Local fixed string";
    struct string_struct str = {local_string, 18, 32};
    
    /* Picture string test */
#ifdef __GNUC__
    cobol_var.data[0] = '1';
#endif
    picture_data.digits[0] = '9';
    
    /* Thread scaled test */
#ifdef _OPENMP
    #pragma omp parallel
    {
        omp_global_var = omp_get_thread_num();
        thread_local_var = omp_get_thread_num() * 10;
    }
#endif
    
    /* Explicit constructor test */
#ifdef __cplusplus
    ExplicitTest expl(42);
    bool is_valid = static_cast<bool>(expl);
    
    /* Optional test */
    optional_var = 100;
    if (optional_var.has_value()) {
        int val = optional_var.value();
    }
    
    /* Mutable test */
    MutableTest mut;
    int count = mut.get_count();
#endif
    
    /* Ordering test */
    for (int i = 0; i < 5; i++) {
        for (int j = 0; j < 5; j++) {
#ifdef __GNUC__
            column_major_array[i][j] = i * 10 + j;
#endif
            multi_array[i % 3][j % 4][0] = i * j;
        }
    }
    
    /* Segment test */
#ifdef __x86_64__
    /* FS/GS access simulation */
    asm volatile("" : : "r"(fs_indirect_ptr), "r"(gs_indirect_ptr));
#endif
    
    /* Prototyped function test */
    int result = prototyped_function(10, 'A', 3.14);
    result = func_ptr(20, 'B', 6.28);
    
    /* Small/packed test */
    small_instance.flag1 = 0;
    small_instance.value = 999;
    
    /* Complex type usage */
    int x = 10, y = 20;
    use_restrict(&x, &y);
    
    static_const_volatile;  /* Reference to prevent optimization */
    thread_specific = 75;
    
    return 0;
}

/* Additional function to generate more debug info */
void helper_function() {
    /* Different scope for variables */
    auto int auto_var = 42;  /* GNU extension */
    volatile int volatile_var = 100;
    
    /* Array in different storage class */
    static int static_array[100];
    
    /* Function with nested blocks */
    {
        int block_local = 50;
        {
            int inner_block = block_local * 2;
            static_array[0] = inner_block;
        }
    }
    
    /* Switch statement for control flow */
    switch (auto_var) {
        case 42:
            volatile_var = 200;
            break;
        default:
            volatile_var = 300;
    }
}

/* Fortran-style common block simulation */
#ifdef __GNUC__
typedef struct {
    int a;
    double b;
    char c[10];
} __attribute__((common_block)) common_block_t;
#endif

/* Variable with alignment attribute */
int __attribute__((aligned(64))) aligned_var;

/* Weak symbol */
int __attribute__((weak)) weak_var = 0;

/* Cleanup attribute */
void cleanup_handler(void *ptr) {
    *(int*)ptr = 0;
}

void function_with_cleanup() {
    int __attribute__((cleanup(cleanup_handler))) auto_clean_var = 100;
    /* auto_clean_var will be cleaned up when going out of scope */
}
