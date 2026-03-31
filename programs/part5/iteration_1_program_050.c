/* test_expr_error.c
 * Designed to trigger error_mark_node return in expr.cc during RTL expansion
 * Compile with: gcc -O1 -c test_expr_error.c -o test.o
 * Or with: gcc -O3 -fno-tree-ccp -fno-tree-forwprop -c test_expr_error.c -o test.o
 */

/* 1. Invalid operations on void expressions */
void void_func(void) {}

/* 2. Misusing __builtin_va_arg */
#include <stdarg.h>

/* 3. Malformed compound literals */
struct BadStruct {
    int valid_field;
};

/* 4. Target-specific expansion failures */
typedef float v4sf __attribute__((vector_size(16)));

/* 5. Transaction Memory constructs (if supported) */
#ifdef __TM_FEATURE__
#define USE_TM 1
#else
#define USE_TM 0
#endif

/* Helper function for variadic misuse */
void misuse_va_arg(void) {
    /* Using __builtin_va_arg without proper va_list initialization */
    /* This should fail during expansion */
    int x = __builtin_va_arg(0, float);  /* Invalid ap argument */
}

/* Complex nested invalid expression */
int nested_invalid(void) {
    /* Multiple layers of invalid operations */
    return sizeof((void)void_func(), 5);  /* sizeof of comma with void left operand */
}

/* Address of invalid compound literal */
void* bad_address(void) {
    /* Taking address of compound literal with invalid designator */
    /* The parser might accept this but expansion should fail */
    return &(struct BadStruct){ .non_existent = 42 };  /* Invalid field designator */
}

/* Vector operations that might fail on some targets */
v4sf vector_test(v4sf a, v4sf b) {
    /* Vector operations might fail expansion on targets without vector support */
    return a + b * a;  /* Complex vector expression */
}

/* Overflow builtins with potentially unsupported types */
long double overflow_test(long double a, long double b) {
    /* __builtin_add_overflow might not support long double on all targets */
    long double result;
    int overflow = __builtin_add_overflow(a, b, &result);
    return result;
}

/* Transactional memory (if compiled with -fgnu-tm) */
int tm_test(int *ptr) {
    int result = 0;
    
    /* __transaction_atomic might fail expansion if TM not supported */
    __transaction_atomic {
        result = *ptr + 1;
    }
    
    return result;
}

/* Main function containing various erroneous constructs */
int main(void) {
    int result = 0;
    
    /* 1. Direct void expression misuse */
    int x = (void)void_func();  /* Casting void to int */
    
    /* 2. Comma operator with void left operand in value context */
    int y = (printf("test"), 10);  /* printf returns int, not void - let's fix this */
    /* Actually, printf returns int, so let's use a true void function */
    int z = (void_func(), 10);  /* This should be problematic */
    
    /* 3. Invalid __builtin_va_arg usage */
    va_list ap;
    /* Not initializing ap properly */
    int w = __builtin_va_arg(ap, float);  /* Type mismatch and uninitialized ap */
    
    /* 4. Complex nested error */
    result += nested_invalid();
    
    /* 5. Attempt to use bad address (might get optimized out) */
    void *ptr = bad_address();
    result += (ptr != 0);
    
    /* 6. Vector operations */
    v4sf v1 = {1.0f, 2.0f, 3.0f, 4.0f};
    v4sf v2 = {5.0f, 6.0f, 7.0f, 8.0f};
    v4sf v3 = vector_test(v1, v2);
    result += (int)v3[0];
    
    /* 7. Overflow builtin with long double */
    long double ld1 = 1.0e100L;
    long double ld2 = 1.0e100L;
    long double ld3 = overflow_test(ld1, ld2);
    result += (int)ld3;
    
    /* 8. Transactional memory */
    int tm_val = 42;
    result += tm_test(&tm_val);
    
    /* 9. More complex invalid expressions */
    /* Using __builtin_constant_p with invalid expression */
    int is_const = __builtin_constant_p((void)0);
    result += is_const;
    
    /* 10. Bit-field address attempt */
    struct {
        unsigned int bitfield : 4;
    } bf = {5};
    
    /* Attempt to take address of bit-field (invalid) */
    unsigned int *bf_ptr = (unsigned int*)&bf.bitfield;  /* Taking address of bit-field */
    result += *bf_ptr;
    
    /* 11. Misaligned pointer with __builtin_assume_aligned */
    char buffer[100];
    int *misaligned = (int*)(buffer + 1);  /* Misaligned pointer */
    int *aligned = __builtin_assume_aligned(misaligned, 16);  /* Assume 16-byte alignment */
    result += aligned[0];
    
    /* 12. Invalid compound literal in non-lvalue context */
    /* This might fail during expansion */
    int *p = &(int){10} + 1;  /* Taking address of compound literal and doing arithmetic */
    result += *p;
    
    return result;
}

/* Additional erroneous constructs outside main */
/* Invalid static initialization with void expression */
static int static_bad = (void_func(), 42);  /* Static init with void expression */

/* Invalid array size with sizeof void expression */
char bad_array[sizeof((void)0)];  /* Array size from sizeof(void expression) */

/* Function with invalid return type deduction */
auto invalid_auto_return() {  /* C++ style auto, but in C this is invalid */
    return (void)0;
}

/* Try to trigger errors in different optimization contexts */
#ifdef __OPTIMIZE__
/* Code that might only fail with optimizations enabled */
int optimized_error(void) {
    /* Complex expression that might fail during aggressive optimization */
    return ((void)0, 1, 2, 3);  /* Multiple commas with void */
}
#endif
