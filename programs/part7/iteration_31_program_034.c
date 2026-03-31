/* Target DWARF attributes through various language constructs */
#include <stdio.h>
#include <stdarg.h>
#include <stdnoreturn.h>

/* External declarations from other modules */
extern void ada_driver(void);
extern void fortran_driver(void);

/* For DW_AT_segment - platform-specific segment attribute */
#ifdef __GNUC__
#define SEGMENT(name) __attribute__((section(#name)))
#else
#define SEGMENT(name)
#endif

/* For DW_AT_threads_scaled */
_Thread_local int thread_local_var = 42;

/* For DW_AT_prototyped - various function prototypes */
void prototyped_func(void);  /* DW_AT_prototyped should be set */
int variadic_func(const char *fmt, ...);  /* Variadic also triggers prototyped */
noreturn void noreturn_func(void);  /* noreturn attribute */

/* For DW_AT_mutable (C++ only) */
#ifdef __cplusplus
class TestClass {
private:
    mutable int mutable_member;  /* DW_AT_mutable */
    int normal_member;
public:
    TestClass() : mutable_member(0), normal_member(0) {}
    void const_method() const {
        mutable_member = 42;  /* Can modify mutable in const method */
    }
};
#endif

/* For DW_AT_location and bounds */
void location_test(void) {
    volatile int vol_var = 1;  /* May affect location encoding */
    const int const_var = 2;
    static int static_var = 3;
    
    /* Array with potential lower bound (Fortran-style thinking) */
    int array[10];
    for (int i = 0; i < 10; i++) {
        array[i] = i * 2;
    }
}

/* For DW_AT_string_length attributes */
struct StringInfo {
    char *data;
    /* Bit-field for string length in bits */
    unsigned int length_bits : 16;  /* DW_AT_string_length_bit_size? */
    unsigned int length_bytes : 16; /* DW_AT_string_length_byte_size? */
};

/* Segment attribute test */
SEGMENT(.special_section) int segment_var = 0xDEADBEEF;

/* Explicit template specialization (C++ for DW_AT_explicit) */
#ifdef __cplusplus
template<typename T>
class ExplicitTest {
public:
    explicit ExplicitTest(T value) {}  /* explicit constructor */
};

template<>
class ExplicitTest<double> {
public:
    explicit ExplicitTest(double value) {}  /* explicit specialization */
};
#endif

/* Optional parameter simulation */
#ifdef __cplusplus
#include <optional>
std::optional<int> optional_value;  /* DW_AT_is_optional? */
#endif

int variadic_func(const char *fmt, ...) {
    va_list args;
    va_start(args, fmt);
    int sum = 0;
    for (int i = 0; i < 5; i++) {
        sum += va_arg(args, int);
    }
    va_end(args);
    return sum;
}

noreturn void noreturn_func(void) {
    while(1);  /* Infinite loop satisfies noreturn */
}

/* Main driver that references everything */
int main(void) {
    int checksum = 0;
    
    /* Reference thread-local variable */
    checksum += thread_local_var;
    
    /* Reference segment variable */
    checksum += segment_var & 0xFF;
    
    /* Call prototyped functions */
    location_test();
    checksum += variadic_func("test", 1, 2, 3, 4, 5);
    
#ifdef __cplusplus
    /* C++ specific tests */
    TestClass obj;
    obj.const_method();
    
    ExplicitTest<int> explicit_obj(42);
    ExplicitTest<double> explicit_dbl(3.14);
    
    optional_value = 42;
    checksum += optional_value.value_or(0);
#endif
    
    /* Call language-specific drivers if available */
#ifdef WITH_ADA
    ada_driver();
#endif
    
#ifdef WITH_FORTRAN
    fortran_driver();
#endif
    
    /* Prevent optimization */
    printf("Checksum: %d\n", checksum);
    return checksum > 0 ? 0 : 1;
}
