/* Test program to trigger generation of specific DWARF attributes */
/* Compile with: gcc -g -dA -O0 -fopenmp -std=c++17 test_dwarf_attrs.c -o test_dwarf_attrs */

#include <stddef.h>
#include <stdint.h>

/* For C++ features */
#ifdef __cplusplus
#include <optional>
#endif

/* ===== DW_AT_string_length, DW_AT_string_length_bit_size, DW_AT_string_length_byte_size ===== */
/* Fixed-length string types that might generate string length attributes */
typedef char fixed_string_32[32];
typedef char fixed_string_64[64];

/* String structure that might trigger length attributes */
struct string_with_len {
    char *data;
    size_t length;
    size_t capacity;
};

/* ===== DW_AT_picture_string ===== */
/* Attempt to trigger picture string attribute using GCC attributes */
#ifdef __GNUC__
/* COBOL/Fortran-style picture data simulation */
struct picture_data {
    char digits[10];
} __attribute__((packed));

/* Try to use picture attribute if supported */
#if defined(__GNUC__) && __GNUC__ >= 7
typedef char picture_string __attribute__((picture("999V99")));
#else
typedef char picture_string[8];
#endif
#endif

/* ===== DW_AT_threads_scaled ===== */
/* OpenMP thread-related variables */
#ifdef _OPENMP
#include <omp.h>
int omp_global_var = 42;
#pragma omp threadprivate(omp_global_var)

/* Variable in OpenMP target region */
int omp_target_var = 100;
#endif

/* ===== DW_AT_explicit ===== */
/* C++ explicit constructor - will only be active in C++ mode */
#ifdef __cplusplus
class ExplicitClass {
public:
    explicit ExplicitClass(int x) : value(x) {}
    explicit operator bool() const { return value != 0; }
private:
    int value;
};

class AnotherExplicit {
public:
    explicit AnotherExplicit(double d) : data(d) {}
private:
    double data;
};
#endif

/* ===== DW_AT_is_optional ===== */
/* C++ optional types */
#ifdef __cplusplus
std::optional<int> global_optional;
std::optional<double> another_optional = 3.14;
#endif

/* ===== DW_AT_mutable ===== */
/* C++ mutable members */
#ifdef __cplusplus
class MutableClass {
public:
    MutableClass() : normal_member(0), mutable_member(0) {}
    void modify() const { mutable_member = 42; }  // Can modify mutable member in const function
private:
    int normal_member;
    mutable int mutable_member;
};

struct MutableStruct {
    mutable int counter;
    const int id;
    MutableStruct(int i) : counter(0), id(i) {}
};
#endif

/* ===== DW_AT_ordering ===== */
/* Array ordering attributes - try column-major for Fortran compatibility */
#ifdef __GNUC__
/* Try column-major attribute if supported */
int column_major_array[5][5]
#ifdef __clang__
    __attribute__((matrix_type(column_major)))
#else
    __attribute__((optimize("no-simd")))
#endif
    ;

/* Another array with different dimensions */
double matrix_3d[3][4][5];
#endif

/* ===== DW_AT_segment ===== */
/* Segment-specific pointers (x86 memory models) */
#ifdef __i386__
/* Far pointers for segmented memory models */
int __attribute__((far)) *far_ptr = NULL;

/* Try to use segment registers if supported */
#if defined(__GNUC__) && defined(__SEG_FS)
    int __seg_fs *fs_ptr = 0;
#endif
#if defined(__GNUC__) && defined(__SEG_GS)
    int __seg_gs *gs_ptr = 0;
#endif
#endif

/* ===== DW_AT_prototyped ===== */
/* Fully prototyped functions */
int prototyped_function(int a, char b, double c);
int (*prototyped_function_ptr)(int, char, double) = &prototyped_function;

/* Another prototyped function with different signature */
void complex_prototype(struct string_with_len *s, int count, ...);
void (*complex_prototype_ptr)(struct string_with_len *, int, ...) = &complex_prototype;

/* ===== DW_AT_small ===== */
/* Packed structures and bit-fields */
struct SmallPacked {
    unsigned int flag : 1;
    unsigned int mode : 3;
    unsigned int : 4;  /* Padding */
    unsigned char data;
} __attribute__((packed));

/* Another small packed structure */
struct TinyStruct {
    unsigned char a : 2;
    unsigned char b : 2;
    unsigned char c : 2;
    unsigned char d : 2;
} __attribute__((packed, aligned(1)));

/* ===== Mixed storage classes and qualifiers ===== */
/* To stress the DIE generation */
static int static_var = 100;
const int const_var = 200;
volatile int volatile_var = 300;
register int register_var asm("eax");

/* Thread-local storage */
__thread int thread_local_var = 400;

/* Restrict pointer */
int *restrict restrict_ptr = NULL;

/* Complex type composition */
typedef int (*complex_func_ptr)(struct SmallPacked *, const char **, ...);
complex_func_ptr func_ptr_array[5];

/* ===== Function implementations ===== */
int prototyped_function(int a, char b, double c) {
    return a + (int)b + (int)c;
}

void complex_prototype(struct string_with_len *s, int count, ...) {
    if (s) {
        s->length = count;
    }
}

/* Main function that uses all types */
int main() {
    /* String length attributes */
    fixed_string_32 str32 = "Test string for length attributes";
    fixed_string_64 str64 = "Another longer test string for DW_AT_string_length attributes";
    
    struct string_with_len dyn_str = {
        .data = str32,
        .length = sizeof(str32),
        .capacity = sizeof(str32)
    };
    
    /* Picture string */
#ifdef __GNUC__
    struct picture_data pic = { .digits = "1234567890" };
    picture_string pic_str = "123.45";
#endif
    
    /* Thread scaled */
#ifdef _OPENMP
    #pragma omp parallel
    {
        omp_global_var = omp_get_thread_num();
        #pragma omp target map(tofrom: omp_target_var)
        {
            omp_target_var++;
        }
    }
#endif
    
    /* Explicit (C++ only) */
#ifdef __cplusplus
    ExplicitClass expl_obj(42);
    ExplicitClass *expl_ptr = &expl_obj;
    
    /* Optional */
    global_optional = 100;
    another_optional.reset();
#endif
    
    /* Mutable (C++ only) */
#ifdef __cplusplus
    MutableClass mut_obj;
    const MutableClass const_mut_obj;
    const_mut_obj.modify();  // Modifies mutable member
    
    MutableStruct mut_struct(1);
    mut_struct.counter++;
#endif
    
    /* Ordering */
    for (int i = 0; i < 5; i++) {
        for (int j = 0; j < 5; j++) {
            column_major_array[i][j] = i * 5 + j;
        }
    }
    
    /* Segment */
#ifdef __i386__
    int local_var = 999;
    far_ptr = &local_var;
#endif
    
    /* Prototyped */
    int result = prototyped_function(10, 'A', 3.14);
    prototyped_function_ptr = NULL;
    
    /* Small */
    struct SmallPacked small = { .flag = 1, .mode = 3, .data = 0xFF };
    struct TinyStruct tiny = { .a = 1, .b = 2, .c = 3, .d = 0 };
    
    /* Mixed storage */
    static_var++;
    volatile_var++;
    thread_local_var++;
    
    if (restrict_ptr) {
        *restrict_ptr = 500;
    }
    
    /* Use function pointer array */
    func_ptr_array[0] = NULL;
    
    /* Return something based on the operations */
    return (result > 0) ? 0 : 1;
}

/* Additional global variables for more coverage */
fixed_string_32 global_string = "Global fixed string";
struct SmallPacked global_small = { .flag = 0, .mode = 7, .data = 0xAA };
int global_array[10][10];

/* Complex nested type */
typedef struct {
    struct {
        int x;
        int y;
    } point;
    struct string_with_len name;
    struct SmallPacked flags;
} ComplexType;

ComplexType global_complex = {
    .point = { .x = 10, .y = 20 },
    .name = { .data = "Complex", .length = 7, .capacity = 32 },
    .flags = { .flag = 1, .mode = 2, .data = 0x55 }
};
