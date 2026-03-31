/* Test program to trigger specific DWARF attributes in dwarf2out.cc */
#include <stdio.h>
#include <stdarg.h>
#include <stdnoreturn.h>

/* For DW_AT_segment */
#ifdef __GNUC__
#define SEGMENT(name) __attribute__((section(#name)))
#else
#define SEGMENT(name)
#endif

/* For thread-local storage */
_Thread_local int thread_var = 42;
thread_local int cpp_thread_var = 100;  /* C++11 style */

/* Global variables in different segments */
SEGMENT(.data.custom) int segment_var = 123;
SEGMENT(.rodata) const int const_segment_var = 456;

/* For DW_AT_mutable (C++ only) */
#ifdef __cplusplus
class TestMutable {
    mutable int mutable_member;
    int normal_member;
public:
    TestMutable() : mutable_member(10), normal_member(20) {}
    void modify() const { mutable_member = 30; }  // Can modify mutable in const
};
#endif

/* For DW_AT_prototyped */
int prototyped_func(int a, int b);  // Declaration without definition
int fully_prototyped(int x, int y) { return x + y; }
int void_prototyped(void) { return 0; }
int variadic_func(const char* fmt, ...);

/* For DW_AT_location variations */
volatile int volatile_var = 789;
const int const_var = 999;

/* For array bounds */
int array_with_bounds[5][10];  // Multi-dimensional
extern int external_array[];   // Incomplete type

/* For DW_AT_string_length attributes */
struct StringStruct {
    char* data;
    int length;
    unsigned bit_field : 4;  // Bit field for bit_size attributes
    unsigned : 4;           // Unnamed bit field
    int normal_field;
} __attribute__((packed));

/* noreturn function */
noreturn void fatal_error(const char* msg) {
    fprintf(stderr, "Error: %s\n", msg);
    while(1);  // Infinite loop instead of abort for test
}

/* Variadic function implementation */
int variadic_func(const char* fmt, ...) {
    va_list args;
    va_start(args, fmt);
    int sum = 0;
    for (int i = 0; i < 3; i++) {
        sum += va_arg(args, int);
    }
    va_end(args);
    return sum;
}

/* Complex type with nested structures */
typedef union {
    struct {
        int lower;  // Could trigger DW_AT_lower_bound
        int upper;
    } bounds;
    double value;
} BoundUnion;

/* Enum with explicit values */
enum Ordering {
    ASCENDING = 1,
    DESCENDING = 2,
    UNORDERED = 0
};

/* Template for DW_AT_explicit (C++ only) */
#ifdef __cplusplus
template<typename T>
class Wrapper {
    T value;
public:
    explicit Wrapper(T v) : value(v) {}  // explicit constructor
    operator T() const { return value; }
};

// Explicit specialization
template<>
class Wrapper<double> {
    double value;
public:
    explicit Wrapper(double v) : value(v) {}
    operator double() const { return value; }
};
#endif

/* Function using many types */
int compute_checksum() {
    int sum = 0;
    
    sum += thread_var;
    sum += cpp_thread_var;
    sum += segment_var;
    sum += volatile_var;
    
    /* Use array with bounds */
    for (int i = 0; i < 5; i++) {
        for (int j = 0; j < 10; j++) {
            array_with_bounds[i][j] = i * j;
            sum += array_with_bounds[i][j];
        }
    }
    
    /* Use bit field structure */
    struct StringStruct s = {0};
    s.bit_field = 7;
    sum += s.bit_field;
    
    /* Use ordering enum */
    enum Ordering ord = ASCENDING;
    sum += ord;
    
    return sum;
}

int main(void) {
    printf("Starting DWARF attribute test...\n");
    
    /* Reference all variables to prevent optimization */
    int checksum = compute_checksum();
    
#ifdef __cplusplus
    /* C++ specific tests */
    TestMutable tm;
    tm.modify();
    
    Wrapper<int> w1(42);
    Wrapper<double> w2(3.14);
    checksum += (int)w1 + (int)w2;
#endif
    
    /* Call variadic function */
    checksum += variadic_func("test", 1, 2, 3);
    
    /* Call fully prototyped functions */
    checksum += fully_prototyped(10, 20);
    checksum += void_prototyped();
    
    printf("Checksum: %d\n", checksum);
    printf("Addresses: %p %p %p\n", 
           (void*)&thread_var, 
           (void*)&segment_var,
           (void*)&volatile_var);
    
    return checksum != 0 ? 0 : 1;
}
