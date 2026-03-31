/* Test program to trigger generation of specific DWARF attributes in GCC */
/* Compile with: gcc -g -dA -O0 test_dwarf_attrs.c -o test_dwarf_attrs */

#include <stddef.h>
#include <string.h>

/* For DW_AT_string_length, DW_AT_string_length_bit_size, DW_AT_string_length_byte_size */
/* Fixed-length string type that might trigger string length attributes */
typedef char fixed_string[32];
fixed_string global_string = "test_string_for_dwarf";

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
int omp_thread_var = 0;
#pragma omp threadprivate(omp_thread_var)
#endif

/* For DW_AT_explicit - C++ explicit constructor (in C we simulate with function attributes) */
/* This would normally be in C++ but we'll use attributes in C */
typedef struct {
    int value;
} ExplicitStruct;

/* For DW_AT_is_optional - optional parameter simulation */
typedef struct {
    int has_value;
    union {
        int value;
        char padding[sizeof(int)];
    };
} OptionalInt;

/* For DW_AT_mutable - mutable data simulation */
typedef struct {
    int regular;
    volatile int mutable_like; /* volatile simulates mutable-like behavior */
} MutableStruct;

/* For DW_AT_ordering - array ordering (column-major simulation) */
#ifdef __GNUC__
int column_major_array[5][5] __attribute__((aligned(16)));
#else
int column_major_array[5][5];
#endif

/* For DW_AT_segment - segment-specific pointers */
/* Use far pointer simulation for segment attributes */
#ifdef __i386__
int * __attribute__((far)) far_pointer = NULL;
#elif defined(__x86_64__)
/* On x86_64, use segment register attributes if available */
#ifdef __GNUC__
int * __attribute__((segment("fs"))) fs_pointer = NULL;
#endif
#endif

/* For DW_AT_prototyped - fully prototyped function */
int fully_prototyped_func(int a, char b, double c) {
    return a + (int)b + (int)c;
}

/* Function pointer to prototyped function */
int (*proto_func_ptr)(int, char, double) = &fully_prototyped_func;

/* For DW_AT_small - packed small types */
struct small_packed {
    unsigned int flag:1;
    unsigned int tiny:3;
} __attribute__((packed));

/* Complex type with multiple attributes */
typedef struct {
    fixed_string name;
    OptionalInt optional_field;
    MutableStruct mutable_field;
    struct small_packed packed_field;
} ComplexType;

/* Thread-local storage for additional thread-related attributes */
#ifdef __GNUC__
__thread int thread_local_var = 42;
#endif

/* Restrict-qualified pointer */
void process_data(int *restrict ptr1, int *restrict ptr2, size_t len) {
    for (size_t i = 0; i < len; i++) {
        ptr1[i] = ptr2[i] * 2;
    }
}

/* Volatile and const combinations */
volatile const int volatile_const_global = 100;
static volatile int static_volatile_var = 200;

/* Array of function pointers */
typedef int (*func_ptr_t)(int, char, double);
func_ptr_t func_array[5] = {&fully_prototyped_func};

/* Nested structures */
struct outer {
    struct inner {
        int a;
        double b;
    } inner_struct;
    char name[20];
};

/* Union with bitfields */
union bitfield_union {
    struct {
        unsigned int a:8;
        unsigned int b:8;
        unsigned int c:8;
        unsigned int d:8;
    } bits;
    unsigned int full;
};

/* Main function using all types */
int main(int argc, char *argv[]) {
    /* String length attributes */
    fixed_string local_string;
    strcpy(local_string, "local_test");
    
    /* Picture string simulation */
    struct picture_data pic = {.data = "PICTURE1234567890"};
    
    /* Optional parameter */
    OptionalInt opt = {.has_value = 1, .value = 42};
    
    /* Mutable-like struct */
    MutableStruct mut = {.regular = 1, .mutable_like = 2};
    mut.mutable_like = 3; /* Simulate mutable modification */
    
    /* Column-major array access */
    for (int i = 0; i < 5; i++) {
        for (int j = 0; j < 5; j++) {
            column_major_array[i][j] = i * 5 + j;
        }
    }
    
    /* Segment pointer */
#ifdef __GNUC__
#ifdef __x86_64__
    /* Initialize fs pointer if possible */
    asm volatile("" : "=m"(fs_pointer));
#endif
#endif
    
    /* Call prototyped function */
    int result = fully_prototyped_func(10, 'A', 3.14);
    
    /* Small packed struct */
    struct small_packed small = {.flag = 1, .tiny = 7};
    
    /* Complex type */
    ComplexType complex = {
        .name = "complex_name",
        .optional_field = {.has_value = 1, .value = 99},
        .mutable_field = {.regular = 5, .mutable_like = 6},
        .packed_field = {.flag = 0, .tiny = 3}
    };
    
    /* Thread operations */
#ifdef _OPENMP
    #pragma omp parallel
    {
        omp_thread_var = omp_get_thread_num();
    }
#endif
    
    /* Use thread-local */
#ifdef __GNUC__
    thread_local_var = argc;
#endif
    
    /* Use restrict pointers */
    int arr1[10], arr2[10];
    for (int i = 0; i < 10; i++) arr2[i] = i;
    process_data(arr1, arr2, 10);
    
    /* Use function pointer array */
    result = func_array[0](1, 'B', 2.71);
    
    /* Nested struct */
    struct outer out = {
        .inner_struct = {.a = 100, .b = 3.14159},
        .name = "outer_name"
    };
    
    /* Bitfield union */
    union bitfield_union u = {.bits = {.a = 1, .b = 2, .c = 3, .d = 4}};
    
    /* Volatile access */
    int volatile_read = volatile_const_global + static_volatile_var;
    
    return result + local_string[0] + opt.value + mut.regular + 
           column_major_array[0][0] + small.flag + complex.optional_field.value +
           out.inner_struct.a + u.bits.a + volatile_read;
}

/* Additional functions with different calling conventions */
#ifdef __GNUC__
void __attribute__((noinline)) noinline_func(void) {
    static_volatile_var++;
}

void __attribute__((always_inline)) inline_func(int x) {
    static_volatile_var += x;
}
#endif

/* Variable with alignment specification */
int __attribute__((aligned(64))) aligned_var = 0;

/* Weak symbol */
#ifdef __GNUC__
int __attribute__((weak)) weak_var = 0;
#endif

/* Cleanup attribute */
#ifdef __GNUC__
void cleanup_handler(int *p) {
    *p = 0;
}

void func_with_cleanup(void) {
    int __attribute__((cleanup(cleanup_handler))) auto_clean = 1;
    /* auto_clean will be cleaned up when leaving scope */
}
#endif
