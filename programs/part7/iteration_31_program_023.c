/* Main driver program targeting specific DWARF attributes */
#include <stdio.h>
#include <stdarg.h>

/* Forward declarations from other modules */
extern void ada_demo(void);
extern void fortran_demo(void);

/* For DW_AT_explicit (C++ specific) */
#ifdef __cplusplus
template<typename T>
class ExplicitDemo {
public:
    explicit ExplicitDemo(T x) : value(x) {}  // explicit constructor
    T value;
};

template<>
class ExplicitDemo<int> {
public:
    explicit ExplicitDemo(int x) : value(x) {}  // explicit specialization
    int value;
};
#endif

/* For DW_AT_mutable (C++ specific) */
#ifdef __cplusplus
class MutableDemo {
    mutable int counter;  // mutable member
public:
    MutableDemo() : counter(0) {}
    void increment() const { counter++; }
};
#endif

/* For DW_AT_prototyped */
int prototyped_func(int a, int b);  // prototype declaration
void variadic_func(const char* fmt, ...);  // variadic prototype
void void_func(void);  // explicit void parameter list
_Noreturn void noreturn_func(void);  // noreturn attribute

/* For DW_AT_location variations */
volatile int volatile_var = 42;
const int const_array[] = {1, 2, 3, 4, 5};

/* For DW_AT_lower_bound */
int array_with_bounds[5] = {0};  // Lower bound is 0 in C

/* For DW_AT_segment (using compiler extension) */
#ifdef __GNUC__
__attribute__((section(".data_segment"))) int segment_var = 100;
__attribute__((section(".code_segment"))) const char* segment_str = "test";
#endif

/* For DW_AT_threads_scaled */
_Thread_local int thread_local_var = 0;
#ifdef __cplusplus
thread_local int thread_local_var_cpp = 0;
#endif

/* For DW_AT_string_length_bit_size and DW_AT_string_length_byte_size */
struct StringInfo {
    char* data;
    unsigned int length_bits : 16;    // bit-field for bit size
    unsigned int length_bytes : 16;   // bit-field for byte size
    unsigned int : 0;                 // force alignment boundary
} __attribute__((packed));

/* For DW_AT_is_optional and DW_AT_small */
struct OptionalSmall {
    int required;
    int optional __attribute__((aligned(1)));  // potentially "small" alignment
} __attribute__((packed));

/* Function definitions */
int prototyped_func(int a, int b) {
    return a + b;
}

void variadic_func(const char* fmt, ...) {
    va_list args;
    va_start(args, fmt);
    vprintf(fmt, args);
    va_end(args);
}

void void_func(void) {
    printf("Void function\n");
}

_Noreturn void noreturn_func(void) {
    printf("No return\n");
    while(1) {}  // infinite loop
}

/* Main driver that references everything */
int main(void) {
    int checksum = 0;
    
    /* Reference C++ constructs if compiling as C++ */
    #ifdef __cplusplus
    ExplicitDemo<double> ed(3.14);
    ExplicitDemo<int> edi(42);
    MutableDemo md;
    md.increment();
    checksum += (int)ed.value + edi.value;
    thread_local_var_cpp = 1;
    #endif
    
    /* Reference thread-local storage */
    thread_local_var = 5;
    checksum += thread_local_var;
    
    /* Reference variables with special attributes */
    checksum += volatile_var;
    checksum += const_array[0];
    checksum += array_with_bounds[0];
    checksum += segment_var;
    
    /* Reference packed structures */
    struct StringInfo si = {"Hello", 40, 5};
    checksum += si.length_bits + si.length_bytes;
    
    struct OptionalSmall os = {1, 2};
    checksum += os.required + os.optional;
    
    /* Call prototyped functions */
    checksum += prototyped_func(10, 20);
    variadic_func("Test: %d\n", checksum);
    void_func();
    
    /* Reference external modules */
    #ifdef HAS_ADA
    ada_demo();
    #endif
    
    #ifdef HAS_FORTRAN
    fortran_demo();
    #endif
    
    printf("Final checksum: %d\n", checksum);
    
    /* Don't call noreturn_func() as it never returns */
    
    return checksum & 0xFF;  // Return non-constant result
}
