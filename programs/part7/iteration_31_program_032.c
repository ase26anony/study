/* Target: DW_AT_explicit, DW_AT_mutable, DW_AT_prototyped, DW_AT_segment */

#include <stdio.h>
#include <stdarg.h>
#include <stdnoreturn.h>

/* Forward declarations from other modules */
extern void ada_test(void);
extern void fortran_test(void);

/* For DW_AT_segment */
#ifdef __GNUC__
#define SEGMENT(name) __attribute__((section(#name)))
#else
#define SEGMENT(name)
#endif

/* For DW_AT_threads_scaled */
_Thread_local int thread_var = 42;

/* For DW_AT_mutable (C++ only) */
#ifdef __cplusplus
class TestClass {
private:
    int regular;
public:
    mutable int mutable_member;  /* Should generate DW_AT_mutable */
    explicit TestClass(int x) : regular(x), mutable_member(x) {}  /* DW_AT_explicit for constructor */
    
    /* Explicit conversion operator */
    explicit operator bool() const { return regular > 0; }
};
#endif

/* For DW_AT_prototyped */
void prototyped_func(int a, char b);  /* Has prototype */
void unprototyped_func();             /* No prototype in C (different from void proto) */
void variadic_func(const char* fmt, ...);  /* Variadic function */

/* noreturn function */
noreturn void fatal_error(const char* msg) {
    fprintf(stderr, "Error: %s\n", msg);
    exit(1);
}

/* Function with segment attribute */
SEGMENT(.special) int segment_var = 100;

/* For DW_AT_location variations */
volatile int vol_var = 1;
const int const_var = 2;

/* Array with potential bounds */
int bounded_array[5] = {1, 2, 3, 4, 5};

/* Bit-field structure for string length attributes */
struct BitFieldStruct {
    unsigned int length : 8;      /* 8-bit field */
    unsigned int : 4;             /* Padding */
    unsigned int byte_size : 12;  /* 12-bit field */
    char data[32];
};

/* Packed structure */
struct __attribute__((packed)) PackedStruct {
    char a;
    int b;
    short c;
};

/* Thread-local with complex type */
_Thread_local struct {
    int id;
    double value;
} thread_struct = {0, 3.14159};

/* Function definitions */
void prototyped_func(int a, char b) {
    printf("prototyped: %d %c\n", a, b);
}

void unprototyped_func() {  /* In C, this has no prototype */
    printf("unprototyped\n");
}

void variadic_func(const char* fmt, ...) {
    va_list args;
    va_start(args, fmt);
    vprintf(fmt, args);
    va_end(args);
}

/* Main driver that references everything */
int main(void) {
    int checksum = 0;
    
    /* Reference thread-local variables */
    checksum += thread_var;
    checksum += thread_struct.id;
    
    /* Reference volatile and const */
    checksum += vol_var;
    checksum += const_var;
    
    /* Reference array with bounds */
    for (int i = 0; i < 5; i++) {
        checksum += bounded_array[i];
    }
    
    /* Reference segment variable */
    checksum += segment_var;
    
    /* Use bit-field structure */
    struct BitFieldStruct bfs = {10, 20, "test"};
    checksum += bfs.length + bfs.byte_size;
    
    /* Use packed structure */
    struct PackedStruct ps = {'A', 123, 456};
    checksum += ps.a + ps.b + ps.c;
    
#ifdef __cplusplus
    /* C++ specific tests */
    TestClass obj(42);
    checksum += obj.mutable_member;
    
    /* Use explicit conversion */
    if (static_cast<bool>(obj)) {
        checksum += 100;
    }
#endif
    
    /* Call functions with different prototypes */
    prototyped_func(10, 'X');
    unprototyped_func();
    variadic_func("Variadic: %d %f\n", 42, 3.14);
    
    /* Call Ada and Fortran tests if available */
    ada_test();
    fortran_test();
    
    /* Print non-constant result */
    printf("Checksum: %d\n", checksum);
    
    /* Prevent optimization */
    volatile int* dummy = &checksum;
    return *dummy % 256;
}
