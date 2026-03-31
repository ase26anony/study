/* Main driver program targeting specific DWARF attributes */
#include <stdio.h>
#include <stdarg.h>

/* External declarations from other modules */
extern void ada_procedure(void);
extern void fortran_subroutine(void);

/* For DW_AT_segment - use segment attribute (GCC extension) */
#ifdef __GNUC__
#define SEGMENT(name) __attribute__((section(#name)))
#else
#define SEGMENT(name)
#endif

/* For DW_AT_threads_scaled - thread-local storage */
_Thread_local int thread_var = 42;

/* For DW_AT_mutable - C++ specific */
#ifdef __cplusplus
class TestClass {
private:
    int normal_member;
public:
    mutable int mutable_member;  /* Should generate DW_AT_mutable */
    
    /* For DW_AT_explicit - explicit constructor */
    explicit TestClass(int x) : normal_member(x), mutable_member(x) {}
    
    /* For DW_AT_prototyped */
    void prototyped_method(int x, ...);  /* Variadic for DW_AT_prototyped */
};

void TestClass::prototyped_method(int x, ...) {
    va_list args;
    va_start(args, x);
    mutable_member += x;
    va_end(args);
}

/* Explicit template specialization */
template<typename T>
class TemplateClass {
public:
    explicit TemplateClass(T val) {}  /* explicit constructor */
};

/* Instantiate with explicit constructor */
TemplateClass<int> explicit_instance(42);
#endif

/* For DW_AT_location - variables with specific storage */
volatile int volatile_var = 1;
const int const_array[] = {1, 2, 3, 4};

/* For DW_AT_lower_bound - array with specified bounds */
int bounded_array[5] = {0};  /* Lower bound is 0 in C */

/* For DW_AT_string_length - string with potential length attributes */
const char* string_with_length = "Hello";

/* Packed structure for bit-size attributes */
struct __attribute__((packed)) PackedStruct {
    unsigned int bit_field : 7;  /* Could influence string length bit size */
    char small_member;           /* For DW_AT_small */
};

/* Function with prototype for DW_AT_prototyped */
void prototyped_function(int a, char b, ...) SEGMENT(.special_section);

void prototyped_function(int a, char b, ...) {
    /* Reference thread-local variable */
    thread_var += a;
}

/* noreturn function */
__attribute__((noreturn)) void no_return_func(void) {
    while(1);  /* Infinite loop */
}

/* Main function that references everything */
int main(void) {
    int checksum = 0;
    
    /* Reference thread-local variable */
    checksum += thread_var;
    
    /* Reference volatile variable */
    checksum += volatile_var;
    
    /* Reference array with bounds */
    for(int i = 0; i < 5; i++) {
        checksum += bounded_array[i];
    }
    
    /* Reference string */
    checksum += string_with_length[0];
    
    /* Call prototyped function */
    prototyped_function(1, 'a', 2, 3);
    
#ifdef __cplusplus
    /* Use C++ features if compiling as C++ */
    TestClass obj(10);
    obj.mutable_member = 20;
    obj.prototyped_method(5, 6, 7);
    checksum += obj.mutable_member;
#endif
    
    /* Call Ada and Fortran procedures if available */
    /* ada_procedure(); */
    /* fortran_subroutine(); */
    
    /* Print checksum to prevent optimization */
    printf("Checksum: %d\n", checksum);
    
    return 0;
}
