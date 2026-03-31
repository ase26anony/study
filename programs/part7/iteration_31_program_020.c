/* main.c - Main driver program */
#include <stdio.h>
#include <stdarg.h>

/* Forward declarations from other modules */
extern void use_complex_types(void);
extern int compute_checksum(void);

/* Global variables with various attributes */
int __attribute__((section(".data"))) global_data = 42;
volatile int volatile_global = 100;
const int const_global = 200;
_Thread_local int thread_local_var = 300;

/* Structure with mutable member (C++) */
#ifdef __cplusplus
class TestClass {
private:
    int regular;
public:
    mutable int mutable_member;  /* Should generate DW_AT_mutable */
    explicit TestClass(int x) : regular(x), mutable_member(x*2) {}  /* explicit constructor */
    
    /* Prototyped functions */
    void prototyped_func(void);  /* Should generate DW_AT_prototyped */
    void variadic_func(const char* fmt, ...);
};
#endif

/* Structure with bit-fields for string length attributes */
struct StringInfo {
    unsigned int length : 16;      /* Bit-sized length */
    unsigned int byte_length : 24; /* Byte-sized length */
    char* data;
};

/* Array with explicit bounds */
int bounded_array[10] = {0,1,2,3,4,5,6,7,8,9};

/* Function with noreturn attribute */
__attribute__((noreturn)) void fatal_error(const char* msg) {
    fprintf(stderr, "Error: %s\n", msg);
    exit(1);
}

/* Variadic function with prototyped declaration */
void log_message(const char* format, ...) {
    va_list args;
    va_start(args, format);
    vprintf(format, args);
    va_end(args);
}

/* Function using segment attribute (GCC extension) */
#ifdef __GNUC__
int __attribute__((section(".mysection"))) segment_var = 999;
#endif

int main(void) {
    /* Reference all globals to prevent optimization */
    int checksum = 0;
    
    checksum += global_data;
    checksum += volatile_global;
    checksum += const_global;
    checksum += thread_local_var;
    
    /* Use the bounded array */
    for (int i = 0; i < 10; i++) {
        checksum += bounded_array[i];
    }
    
#ifdef __cplusplus
    TestClass obj(checksum);
    obj.mutable_member = checksum * 2;
    checksum += obj.mutable_member;
#endif
    
    /* Use complex types from other module */
    use_complex_types();
    checksum += compute_checksum();
    
#ifdef __GNUC__
    checksum += segment_var;
#endif
    
    /* Create string info structure */
    struct StringInfo str_info = {256, 1024, "Hello"};
    checksum += str_info.length;
    checksum += str_info.byte_length;
    
    /* Call variadic function */
    log_message("Checksum: %d\n", checksum);
    
    /* Prevent dead code elimination */
    if (checksum > 10000) {
        fatal_error("Impossible condition");
    }
    
    return checksum & 0xFF;
}
