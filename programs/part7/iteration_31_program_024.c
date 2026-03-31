/* Target DWARF attributes through various language constructs */
#include <stdio.h>
#include <stdarg.h>
#include <stdnoreturn.h>

/* External declarations from other modules */
extern void ada_driver(void);
extern void fortran_driver(void);

/* For DW_AT_explicit (C++ only) */
#ifdef __cplusplus
template<typename T>
class ExplicitTest {
public:
    explicit ExplicitTest(T value) : data(value) {}  // Should generate DW_AT_explicit
    T data;
};
#endif

/* For DW_AT_mutable (C++ only) */
#ifdef __cplusplus
class MutableTest {
    mutable int counter;  // Should generate DW_AT_mutable
public:
    MutableTest() : counter(0) {}
    void increment() const { counter++; }
};
#endif

/* For DW_AT_location and DW_AT_segment */
#ifdef __GNUC__
__attribute__((section(".mysegment")))
#endif
volatile int segment_var = 42;  // Should generate DW_AT_segment

/* For DW_AT_prototyped */
int prototyped_func(int a, int b);  // Declaration without definition
void variadic_func(const char* fmt, ...);  // Variadic function

/* For DW_AT_threads_scaled */
_Thread_local int thread_local_var = 0;

/* For DW_AT_string_length_bit_size and DW_AT_string_length_byte_size */
struct StringInfo {
    char data[100];
    unsigned int length : 7;  // Bit field for string length
    unsigned int byte_length;
};

/* For DW_AT_lower_bound */
int array_with_bounds[5] = {1, 2, 3, 4, 5};

/* For DW_AT_is_optional (simulated via pointer) */
struct OptionalData {
    int* optional_value;  // NULL means optional
    int required_value;
};

/* For DW_AT_ordering (enum ordering) */
enum OrderedEnum {
    FIRST = 1,
    SECOND = 2,
    THIRD = 3
};

/* For DW_AT_small (packed structure) */
struct __attribute__((packed)) SmallStruct {
    char a;
    int b;
    char c;
};

/* noreturn function for DW_AT_prototyped */
noreturn void fatal_error(const char* msg) {
    fprintf(stderr, "Error: %s\n", msg);
    while(1);  // Infinite loop instead of exit for noreturn demonstration
}

/* Function using variadic arguments */
void variadic_func(const char* fmt, ...) {
    va_list args;
    va_start(args, fmt);
    vprintf(fmt, args);
    va_end(args);
}

/* Main driver that references all constructs */
int main() {
    /* Prevent optimization */
    volatile int checksum = 0;
    
    /* Reference thread-local variable */
    thread_local_var = 1;
    checksum += thread_local_var;
    
    /* Reference segment variable */
    checksum += segment_var;
    
    /* Reference array with bounds */
    for (int i = 0; i < 5; i++) {
        checksum += array_with_bounds[i];
    }
    
    /* Reference bit-field structure */
    struct StringInfo str = {"Hello", 5, 5};
    checksum += str.length;
    
    /* Reference packed structure */
    struct SmallStruct small = {'a', 42, 'b'};
    checksum += small.b;
    
    /* Reference optional structure */
    struct OptionalData opt = {NULL, 99};
    checksum += opt.required_value;
    
    /* Call variadic function */
    variadic_func("Checksum so far: %d\n", checksum);
    
#ifdef __cplusplus
    /* C++ specific tests */
    ExplicitTest<int> explicit_obj(42);
    checksum += explicit_obj.data;
    
    MutableTest mutable_obj;
    mutable_obj.increment();
#endif
    
    /* Call external drivers for Ada/Fortran specific attributes */
    ada_driver();
    fortran_driver();
    
    /* Print final checksum to prevent dead code elimination */
    printf("Final checksum: %d\n", checksum);
    
    return checksum > 0 ? 0 : 1;
}
