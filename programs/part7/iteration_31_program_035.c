/* test_dwarf_attributes.c - Main C test file */
#include <stdio.h>
#include <stdarg.h>

/* For DW_AT_segment */
#ifdef __GNUC__
#define SEGMENT(name) __attribute__((section(#name)))
#else
#define SEGMENT(name)
#endif

/* For thread-local storage (DW_AT_threads_scaled) */
_Thread_local int thread_local_var = 42;

/* Segment-specific variable */
int segment_var SEGMENT(.my_segment) = 100;

/* Structure with various attributes */
struct ComplexStruct {
    /* DW_AT_mutable in C++ context */
    int regular_member;
    const int const_member;
    volatile int volatile_member;
    
    /* Bit-fields for string length attributes */
    struct {
        unsigned int length_bits : 7;  /* Could relate to DW_AT_string_length_bit_size */
        unsigned int length_bytes : 8; /* Could relate to DW_AT_string_length_byte_size */
    } string_info;
    
    /* Array with explicit bounds */
    int bounded_array[10];
};

/* Function prototypes for DW_AT_prototyped */
void prototyped_func(void);  /* Explicit void parameter list */
int variadic_func(const char *fmt, ...);  /* Variadic function */
void noreturn_func(void) __attribute__((noreturn));

/* Global with ordering attribute (DW_AT_ordering) */
enum Ordering {
    ASCENDING,
    DESCENDING,
    UNORDERED
} global_ordering = ASCENDING;

/* Structure with small attribute hint */
struct SmallStruct {
    char a, b, c;
} __attribute__((packed));

/* Function using location attribute */
void use_location(struct ComplexStruct *cs) {
    /* Force location tracking */
    cs->volatile_member = cs->regular_member + 1;
}

/* Variadic function implementation */
int variadic_func(const char *fmt, ...) {
    va_list args;
    int sum = 0;
    
    va_start(args, fmt);
    while (*fmt) {
        if (*fmt == 'd') {
            sum += va_arg(args, int);
        }
        fmt++;
    }
    va_end(args);
    
    return sum;
}

/* Noreturn function */
void noreturn_func(void) {
    while(1);  /* Infinite loop */
}

/* Main function that references everything */
int main(void) {
    struct ComplexStruct cs = {
        .regular_member = 10,
        .const_member = 20,
        .volatile_member = 30,
        .string_info = { .length_bits = 64, .length_bytes = 8 },
        .bounded_array = {0}
    };
    
    struct SmallStruct ss = {'x', 'y', 'z'};
    
    /* Reference thread-local variable */
    int tl_value = thread_local_var;
    
    /* Reference segment variable */
    int seg_value = segment_var;
    
    /* Use location */
    use_location(&cs);
    
    /* Call variadic function */
    int var_sum = variadic_func("ddd", 1, 2, 3);
    
    /* Calculate checksum to prevent optimization */
    unsigned long checksum = 0;
    checksum += (unsigned long)&cs;
    checksum += (unsigned long)&ss;
    checksum += tl_value;
    checksum += seg_value;
    checksum += var_sum;
    checksum += global_ordering;
    checksum += sizeof(cs.bounded_array);  /* Array size */
    
    /* Reference string length attributes */
    checksum += cs.string_info.length_bits;
    checksum += cs.string_info.length_bytes;
    
    printf("Checksum: %lu\n", checksum);
    
    return (int)(checksum % 256);
}
