/* test_expr_error.c - Test program to trigger error_mark_node in expr.cc */

/* Include for printf declaration */
#include <stdio.h>
#include <stdarg.h>

/* ====== 1. Invalid Operations on void ====== */
void void_func(void) {}

/* ====== 2. Vector types for potential unsupported operations ====== */
typedef int v4si __attribute__((vector_size(16)));

/* ====== 3. Variadic function for va_arg misuse ====== */
void variadic_func(int n, ...) {
    va_list ap;
    va_start(ap, n);
    /* Intentional misuse below in main() */
    va_end(ap);
}

/* ====== 4. Struct for malformed compound literals ====== */
struct bad_struct {
    int valid_field;
    /* Note: no 'non_existent_field' exists */
};

/* ====== 5. Function using __builtin_add_overflow with questionable types ====== */
int test_overflow(void) {
    int res, overflow;
    /* This should work, but similar patterns with unsupported types might fail */
    overflow = __builtin_add_overflow(1, 2, &res);
    return res;
}

/* ====== 6. Transaction Memory function (if supported) ====== */
#ifdef __TM_FUNCTIONS
int tm_func(int *ptr) __transaction_atomic {
    return *ptr + 1;
}
#endif

/* ====== MAIN FUNCTION WITH ERROR-TRIGGERING CONSTRUCTS ====== */
int main(void) {
    int result = 0;
    
    /* Attempt 1: Using void expression in value context */
    /* This should trigger error during expansion */
    int x = (void)void_func();
    
    /* Attempt 2: Comma operator with void left side */
    int y = (printf("test"), 5);
    
    /* Attempt 3: Misusing __builtin_va_arg outside proper context */
    /* Create a va_list but use it incorrectly */
    va_list ap;
    /* Note: ap is not initialized here - this is deliberate misuse */
    float f = __builtin_va_arg(ap, float);
    
    /* Attempt 4: Malformed compound literal */
    /* Using non-existent field in designated initializer */
    int *p = &(int){ .non_existent_field = 1 };
    
    /* Attempt 5: Nested invalid operations */
    /* sizeof of a void expression */
    size_t sz = sizeof((void)0);
    
    /* Attempt 6: __builtin_constant_p with invalid expression */
    int is_const = __builtin_constant_p((void)0);
    
    /* Attempt 7: Vector operations (may fail on targets without vector support) */
    v4si a = {1, 2, 3, 4};
    v4si b = {5, 6, 7, 8};
    v4si c = a + b;  /* Vector addition */
    
    /* Attempt 8: Complex type with overflow builtin */
    /* Some targets might not support long double overflow checking */
    long double ld1 = 1.0e100L, ld2 = 1.0e100L;
    long double ld_res;
    /* Note: __builtin_add_overflow doesn't work with long double directly,
       but we can try to create a similar problematic pattern */
    
    /* Attempt 9: Invalid address operations */
    /* Taking address of a bit-field (if we had one) */
    struct {
        unsigned int bitfield : 4;
    } bit_struct = {0};
    /* int *bit_ptr = &bit_struct.bitfield; */ /* Would be error, but caught earlier */
    
    /* Attempt 10: Misaligned pointer in alignment context */
    char buffer[10];
    int *misaligned = (int*)(buffer + 1);
    int *aligned = __builtin_assume_aligned(misaligned, 4);
    
    /* Attempt 11: Transaction Memory (if compiled with -fgnu-tm) */
    #ifdef __TM_FUNCTIONS
    int val = 42;
    int tm_result = tm_func(&val);
    #endif
    
    /* Attempt 12: Using variadic function incorrectly */
    /* Passing wrong type to va_arg */
    variadic_func(3, 1, 2.0, "three");
    
    /* The program doesn't need to run successfully */
    /* Just need to trigger compiler errors during expansion */
    return 0;
}

/* ====== ADDITIONAL ERROR-TRIGGERING PATTERNS ====== */
/* External linkage to potentially confuse the compiler */
extern int undefined_extern;

int another_function(void) {
    /* Using undefined external in expression */
    return undefined_extern + 1;
}

/* Complex expression with multiple potential issues */
int complex_error_expr(void) {
    /* Chain of operations where one might fail during expansion */
    return (sizeof((void)void_func()) + __builtin_constant_p((void)0));
}

/* Function with invalid return type expression */
int invalid_return(void) {
    return (void)void_func();
}

/* Statement expression with void result in value context */
int statement_expr_test(void) {
    return ({ void_func(); 5; });
}
