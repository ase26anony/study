/* test_dwarf_attrs.c - Comprehensive test for triggering specific DWARF attributes */
/* Compile with: gcc -g -O0 -dA -fopenmp -std=c++17 test_dwarf_attrs.c -o test_dwarf_attrs */

#include <stddef.h>
#include <stdint.h>

/* For DW_AT_string_length, DW_AT_string_length_bit_size, DW_AT_string_length_byte_size */
/* Fixed-length string type - may trigger string length attributes */
typedef char fixed_string[32];
fixed_string global_string = "test_string_for_dwarf";

/* For DW_AT_picture_string - COBOL/Fortran PICTURE data simulation */
/* Use GCC attribute if available */
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

/* Thread-local storage */
__thread int thread_local_var = 42;
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

/* For DW_AT_is_optional - Optional types */
#ifdef __cplusplus
#include <optional>
std::optional<int> optional_var = 42;
#else
/* C alternative using pointers */
typedef struct {
    int has_value;
    int value;
} optional_int;
optional_int c_optional = {1, 42};
#endif

/* For DW_AT_mutable - Mutable member */
#ifdef __cplusplus
class MutableClass {
public:
    MutableClass() : counter(0) {}
    int get_counter() const { 
        /* mutable can be modified in const method */
        return ++counter; 
    }
private:
    mutable int counter;
};
#endif

/* For DW_AT_ordering - Array ordering (column-major) */
#ifdef __GNUC__
int column_major_array[5][5] __attribute__((aligned(16)));
#else
int column_major_array[5][5];
#endif

/* For DW_AT_segment - Segment-specific pointers */
/* Try to use segment attributes if available */
#ifdef __i386__
#ifdef __GNUC__
int __seg_fs *fs_seg_ptr = 0;
int __seg_gs *gs_seg_ptr = 0;
#endif
#endif

/* For DW_AT_prototyped - Function with prototype */
int prototyped_function(int a, char b, double c);
int (*func_ptr)(int, char, double) = &prototyped_function;

/* For DW_AT_small - Packed small types */
struct SmallPacked {
    unsigned int flag : 1;
    unsigned int tiny : 3;
} __attribute__((packed));

/* Complex type compositions to stress DIE generation */
typedef struct {
    fixed_string name;
    int id;
    struct picture_data *pic;
} ComplexType;

/* Variables with different storage classes */
static int static_var = 100;
volatile int volatile_var = 200;
const int const_var = 300;
register int register_var asm("eax");

/* Function definitions */
int prototyped_function(int a, char b, double c) {
    return a + (int)b + (int)c;
}

/* Main test function */
int main() {
    /* Use all variables to ensure they're not optimized away */
    fixed_string local_string = "local";
    
    struct picture_data pic_local = {"PICTURE_DATA"};
    
    #ifdef _OPENMP
    #pragma omp parallel
    {
        omp_thread_var = omp_get_thread_num();
        thread_local_var = omp_get_thread_num() * 10;
    }
    #endif
    
    #ifdef __cplusplus
    ExplicitClass expl_obj(42);
    MutableClass mut_obj;
    int counter_val = mut_obj.get_counter();
    #endif
    
    /* Use arrays with different access patterns */
    for (int i = 0; i < 5; i++) {
        for (int j = 0; j < 5; j++) {
            column_major_array[i][j] = i * 5 + j;
        }
    }
    
    struct SmallPacked small = {1, 7};
    
    ComplexType complex = {
        .name = "complex_name",
        .id = 999,
        .pic = &pic_local
    };
    
    /* Call function through pointer */
    int result = func_ptr(10, 'A', 3.14);
    
    /* Use all variables to prevent optimization */
    volatile int dummy = 
        static_var + 
        volatile_var + 
        const_var + 
        result +
        small.flag +
        complex.id;
    
    return dummy > 0 ? 0 : 1;
}

/* Additional functions with various prototypes */
static int static_func(int x) { return x * 2; }
extern int extern_func(void);
inline int inline_func(int x) { return x + 1; }

/* Variable with asm label */
int asm_var asm("custom_asm_name") = 1234;

/* Union with bitfields */
union BitfieldUnion {
    struct {
        unsigned int a : 4;
        unsigned int b : 4;
        unsigned int c : 8;
    } bits;
    unsigned int full;
};

/* Enum with explicit values */
enum ExplicitEnum {
    ENUM_A = 1,
    ENUM_B = 2,
    ENUM_C = 4,
    ENUM_D = 8
};
