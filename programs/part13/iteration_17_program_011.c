/* test_dwarf_attrs.c - Comprehensive test for DWARF attribute generation */
/* Compile with: gcc -g -dA -O0 -fopenmp -std=c++17 test_dwarf_attrs.c -o test_dwarf_attrs */

#include <stddef.h>
#include <stdint.h>

/* For DW_AT_string_length, DW_AT_string_length_bit_size, DW_AT_string_length_byte_size */
/* Fixed-length string type (simulating Fortran CHARACTER) */
typedef char fixed_string[32];
fixed_string global_string = "Test string for length attributes";

/* String structure with explicit length */
struct string_with_len {
    char *data;
    size_t length;
    size_t capacity;
};

/* For DW_AT_picture_string - COBOL-style picture data */
/* Using GCC attribute if available */
#ifdef __GNUC__
struct cobol_picture {
    char data[20];
} __attribute__((packed));
#else
#pragma pack(push, 1)
struct cobol_picture {
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

/* For DW_AT_is_optional - Optional type simulation */
/* In C, we simulate with a tagged union */
struct optional_int {
    int has_value;
    int value;
};

/* For DW_AT_mutable - Mutable member */
#ifdef __cplusplus
class ClassWithMutable {
public:
    ClassWithMutable() : counter(0) {}
    int get_counter() const { 
        /* mutable member can be modified in const method */
        return ++counter; 
    }
private:
    mutable int counter;
};
#endif

/* For DW_AT_ordering - Column-major array */
#ifdef __GNUC__
int column_major_array[5][5] __attribute__((column_major));
#else
/* Fallback - just declare the array */
int column_major_array[5][5];
#endif

/* For DW_AT_segment - Segment-specific pointers */
/* Using compiler-specific segment attributes */
#ifdef __i386__
#ifdef __GNUC__
int __seg_fs *fs_seg_ptr;
int __seg_gs *gs_seg_ptr;
#endif
#endif

/* For DW_AT_prototyped - Function with prototype */
int prototyped_function(int a, char b, double c);
int (*func_ptr)(int, char, double) = &prototyped_function;

/* For DW_AT_small - Packed structure with bit-fields */
struct small_packed {
    unsigned int flag1 : 1;
    unsigned int flag2 : 1;
    unsigned int flag3 : 1;
    unsigned int : 5;  /* padding */
    unsigned char small_data;
} __attribute__((packed));

/* Complex type compositions to stress DIE generation */
typedef int (*complex_func_ptr)(int, ...);
typedef struct string_with_len *(*factory_func)(size_t);

/* Variables with different storage classes */
static int static_var = 100;
volatile int volatile_var = 200;
const int const_var = 300;
register int register_var asm("ebx");  /* Hint for register storage */

/* Function implementations */
int prototyped_function(int a, char b, double c) {
    return a + (int)b + (int)c;
}

struct string_with_len *create_string(size_t len) {
    struct string_with_len *str = (struct string_with_len*)malloc(sizeof(struct string_with_len));
    if (str) {
        str->data = (char*)malloc(len + 1);
        str->length = 0;
        str->capacity = len;
        if (str->data) {
            str->data[0] = '\0';
        }
    }
    return str;
}

/* Main test function */
int main() {
    /* Test string length attributes */
    fixed_string local_string = "Local test string";
    struct string_with_len *dynamic_str = create_string(50);
    
    /* Test picture string */
    struct cobol_picture picture_data = {"12345678901234567890"};
    
    /* Test OpenMP thread scaling */
    #ifdef _OPENMP
    #pragma omp parallel
    {
        omp_thread_var = omp_get_thread_num();
        thread_local_var = omp_get_thread_num() * 100;
    }
    #endif
    
    /* Test explicit constructor (C++ only) */
    #ifdef __cplusplus
    ExplicitClass expl_obj(42);
    ClassWithMutable mutable_obj;
    int counter_val = mutable_obj.get_counter();
    #endif
    
    /* Test optional type */
    struct optional_int opt = {1, 999};
    
    /* Test column-major array access */
    for (int i = 0; i < 5; i++) {
        for (int j = 0; j < 5; j++) {
            column_major_array[i][j] = i * 10 + j;
        }
    }
    
    /* Test small packed structure */
    struct small_packed small = {1, 0, 1, 0xAB};
    
    /* Test function pointer */
    int result = func_ptr(10, 'A', 3.14);
    
    /* Use volatile variable to prevent optimization */
    volatile_var = result + static_var + const_var;
    
    /* Cleanup */
    if (dynamic_str) {
        free(dynamic_str->data);
        free(dynamic_str);
    }
    
    return 0;
}

/* Additional complex declarations at file scope */
/* Array of function pointers */
int (*func_array[5])(int, char, double);

/* Nested structure with bitfields */
struct nested_complex {
    struct {
        unsigned int a : 3;
        unsigned int b : 5;
        unsigned int c : 8;
    } inner;
    union {
        int x;
        float y;
        void *z;
    } data;
    enum { RED, GREEN, BLUE } color;
};

/* Template for C++ (if compiled as C++) */
#ifdef __cplusplus
template<typename T>
class TemplateClass {
public:
    TemplateClass(T val) : value(val) {}
    T get_value() const { return value; }
private:
    T value;
};

/* Instantiate template */
TemplateClass<int> template_instance(12345);
#endif

/* Variable with alignment specification */
int aligned_var __attribute__((aligned(64)));

/* Weak symbol */
int weak_var __attribute__((weak)) = 9999;

/* Cleanup function with destructor attribute */
void cleanup_func() __attribute__((destructor));
void cleanup_func() {
    /* Cleanup code */
}
