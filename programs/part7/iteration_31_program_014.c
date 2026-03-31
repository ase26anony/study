/* test_debug_attributes.c - Main C test file */
#include <stdio.h>
#include <stdarg.h>

/* For DW_AT_segment */
#ifdef __GNUC__
#define SEGMENT_ATTR __attribute__((section(".mysegment")))
#else
#define SEGMENT_ATTR
#endif

/* For DW_AT_threads_scaled */
_Thread_local int thread_local_var = 42;

/* Global with segment attribute */
SEGMENT_ATTR int segment_var = 100;

/* Structure with various attributes */
struct ComplexStruct {
    int normal;
    volatile int volatile_member;  /* May affect DW_AT_location */
    const int const_member;
    mutable int mutable_member;    /* C++: DW_AT_mutable */
    
    /* Bit-fields for string length attributes */
    unsigned int length_bits : 7;  /* DW_AT_string_length_bit_size */
    unsigned int length_bytes : 24; /* DW_AT_string_length_byte_size */
    
    /* Array with explicit bounds */
    int bounded_array[10];         /* DW_AT_lower_bound */
    
    /* Small attribute candidate */
    char small_member;
} __attribute__((packed));

/* Function with prototype (DW_AT_prototyped) */
int prototyped_func(int a, int b, ...) {
    va_list args;
    va_start(args, b);
    int c = va_arg(args, int);
    va_end(args);
    return a + b + c;
}

/* Function without prototype (for contrast) */
int old_style_func();  /* Declaration only */

/* Variadic function */
void variadic_func(const char* fmt, ...) {
    /* Empty for test */
}

/* Noreturn function */
#ifdef __GNUC__
__attribute__((noreturn))
#endif
void noreturn_func() {
    while(1) {}
}

/* Enum with ordering */
enum OrderedEnum {
    FIRST = 1,
    SECOND = 2,
    THIRD = 3
};

/* Union for complex type */
union ComplexUnion {
    int as_int;
    float as_float;
    struct ComplexStruct as_struct;
};

/* External declarations from other modules */
extern void external_func(void);
extern int external_array[];

/* Picture string simulation (Ada-like) */
typedef struct {
    const char* picture;
    int scale;
    int currency;
} PictureString;

/* Optional parameter simulation */
#ifdef __cplusplus
template<typename T>
class Optional {
    T value;
    bool has_value;
public:
    explicit Optional(T val) : value(val), has_value(true) {}
    Optional() : has_value(false) {}
    bool is_optional() const { return !has_value; }
};
#endif

/* Main function references everything */
int main() {
    /* Local variables with various types */
    struct ComplexStruct cs = {
        .normal = 1,
        .volatile_member = 2,
        .const_member = 3,
        .mutable_member = 4,
        .length_bits = 127,
        .length_bytes = 1000,
        .small_member = 'x'
    };
    
    /* Initialize array */
    for (int i = 0; i < 10; i++) {
        cs.bounded_array[i] = i * 2;
    }
    
    union ComplexUnion cu;
    cu.as_int = 42;
    
    enum OrderedEnum oe = SECOND;
    
    PictureString ps = {"$$$,$$9.99", 2, 840};
    
    /* Use thread-local variable */
    thread_local_var += 1;
    
    /* Use segment variable */
    segment_var *= 2;
    
    /* Call functions */
    int result = prototyped_func(1, 2, 3);
    variadic_func("Test %d %f", 42, 3.14);
    
    /* Reference external symbols */
    external_func();
    int ext_val = external_array[0];
    
    /* Calculate checksum to prevent optimization */
    unsigned long checksum = 0;
    checksum += (unsigned long)&cs;
    checksum += cs.normal;
    checksum += cs.volatile_member;
    checksum += cs.length_bits;
    checksum += cs.length_bytes;
    checksum += thread_local_var;
    checksum += segment_var;
    checksum += result;
    checksum += (int)oe;
    checksum += ext_val;
    
    printf("Checksum: %lu\n", checksum);
    
    return (int)(checksum % 256);
}

/* Old-style function definition */
int old_style_func(a, b)
    int a;
    float b;
{
    return a + (int)b;
}
