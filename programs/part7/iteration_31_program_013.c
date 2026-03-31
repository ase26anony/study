/* Test program to trigger specific DWARF attribute generation */
#include <stdio.h>
#include <stdarg.h>
#include <stdnoreturn.h>

/* For DW_AT_segment - use compiler-specific segment attribute */
#ifdef __GNUC__
#define SEGMENT(name) __attribute__((section(#name)))
#else
#define SEGMENT(name)
#endif

/* For thread-local storage - DW_AT_threads_scaled */
_Thread_local int thread_var = 42;
_Thread_local long thread_array[10];

/* Global variables with various attributes */
volatile int volatile_var = 100;
const int const_array[] = {1, 2, 3, 4, 5};

/* Packed structure for bit-size attributes */
struct __attribute__((packed)) PackedStruct {
    unsigned int length : 12;  /* Could influence DW_AT_string_length_bit_size */
    unsigned int size : 4;
    char data[10];
};

/* Array with explicit bounds - DW_AT_lower_bound */
int bounded_array[5] = {0};
extern int extern_array[];

/* Function prototypes - DW_AT_prototyped */
void prototyped_func(void);
int variadic_func(int count, ...);
noreturn void noreturn_func(void);

/* Structure with mutable-like behavior (C doesn't have mutable, but use volatile) */
struct TestStruct {
    int normal;
    volatile int mutable_like;  /* Simulate DW_AT_mutable */
    const int read_only;
};

/* Complex type with ordering hint */
enum OrderedEnum {
    FIRST = 0,
    SECOND,
    THIRD,
    LAST = 255
};

/* External declarations to force debug info generation */
extern struct PackedStruct external_packed;
extern void dummy_extern(void);

/* Thread-local with scaling */
_Thread_local double scaled_thread_var[100];

/* Function using segment attribute */
SEGMENT(.special) int segment_var = 999;

/* Variadic function implementation */
int variadic_func(int count, ...) {
    va_list args;
    int sum = 0;
    
    va_start(args, count);
    for (int i = 0; i < count; i++) {
        sum += va_arg(args, int);
    }
    va_end(args);
    
    return sum;
}

noreturn void noreturn_func(void) {
    while (1) {
        /* Infinite loop - noreturn */
    }
}

void prototyped_func(void) {
    /* Empty but prototyped */
}

/* Function that uses all types to prevent optimization */
unsigned long use_all_types(void) {
    struct PackedStruct ps = {0};
    struct TestStruct ts = {0};
    unsigned long checksum = 0;
    
    /* Use thread-local variables */
    checksum += thread_var;
    checksum += (unsigned long)&thread_array[0];
    
    /* Use volatile and const */
    checksum += volatile_var;
    checksum += const_array[0];
    
    /* Use bounded array */
    for (int i = 0; i < 5; i++) {
        checksum += bounded_array[i];
    }
    
    /* Use packed structure */
    ps.length = 100;
    checksum += ps.length;
    
    /* Use test structure */
    ts.normal = 50;
    ts.mutable_like = 25;  /* Volatile access */
    checksum += ts.normal + ts.mutable_like;
    
    /* Use ordered enum */
    checksum += SECOND;
    
    /* Use segment variable */
    checksum += segment_var;
    
    /* Use scaled thread variable */
    checksum += (unsigned long)&scaled_thread_var[0];
    
    /* Call variadic function */
    checksum += variadic_func(3, 10, 20, 30);
    
    return checksum;
}

int main(void) {
    unsigned long result = use_all_types();
    
    /* Print result to prevent dead code elimination */
    printf("Debug attribute test checksum: %lu\n", result);
    printf("Size of PackedStruct: %zu\n", sizeof(struct PackedStruct));
    printf("Address of thread_var: %p\n", (void*)&thread_var);
    printf("Address of segment_var: %p\n", (void*)&segment_var);
    
    return 0;
}
