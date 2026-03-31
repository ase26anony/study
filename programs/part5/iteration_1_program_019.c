/* test_expr_error.c - Trigger error_mark_node in expr.cc during RTL expansion */

/* Force certain compiler behaviors */
#pragma GCC optimize ("O3")
#pragma GCC push_options

/* Test 1: Invalid operations on void expressions */
void void_func(void) {}

/* Test 2: Misusing __builtin_va_arg */
#include <stdarg.h>

/* Test 3: Malformed compound literals */
struct BadStruct {
    int valid_field;
};

/* Test 4: Vector extensions on non-vector targets */
typedef int v4si __attribute__((vector_size(16)));

/* Test 5: Transaction Memory without support */
#ifdef __GNUC__
#define TRANSACTION __transaction_atomic
#else
#define TRANSACTION
#endif

/* Test 6: Complex type mismatches with builtins */
typedef _Complex float complex_float;

/* Helper function for variadic misuse */
void misuse_va_arg(void) {
    /* This should fail during expansion - using va_arg without va_start */
    va_list ap;
    /* Uninitialized ap used with mismatched type */
    int x = __builtin_va_arg(ap, float);  /* float vs int mismatch */
}

/* Main container for erroneous constructs */
int main(void) {
    /* Test Case A: Direct void expression misuse */
    /* This should trigger error during expansion when trying to get value */
    int x = (void)void_func();
    
    /* Test Case B: Void in complex expression context */
    /* The comma operator yields void, but we try to use it as value */
    int y = (printf("test"), 5);
    
    /* Test Case C: Invalid compound literal */
    /* Non-existent field in designated initializer */
    int *p = &(int){ .non_existent_field = 1 };
    
    /* Test Case D: Compound literal in invalid context */
    /* Taking address of non-lvalue compound literal in sizeof */
    size_t s = sizeof(&(int){1, 2, 3});
    
    /* Test Case E: Vector operations (may fail on non-vector targets) */
    v4si a = {1, 2, 3, 4};
    v4si b = {5, 6, 7, 8};
    v4si c = a + b;  /* Vector addition might fail expansion */
    
    /* Test Case F: Transaction Memory (if compiled with -fgnu-tm) */
    TRANSACTION {
        x = 42;
    }
    
    /* Test Case G: Overflow builtin with unsupported type */
    /* long double overflow check might fail on some targets */
    int overflow;
    __builtin_add_overflow(1.0L, 2.0L, &overflow);
    
    /* Test Case H: Complex type with __builtin_constant_p */
    /* Passing void expression to builtin */
    int is_const = __builtin_constant_p((void)0);
    
    /* Test Case I: Nested invalid operations */
    /* Conditional operator with void in one branch */
    int z = (x > 0) ? (void)void_func() : 5;
    
    /* Test Case J: Misaligned pointer assumption */
    char buffer[10];
    int *misaligned = (int*)(buffer + 1);
    int *aligned = __builtin_assume_aligned(misaligned, 4);
    
    /* Test Case K: Bit-field address attempt */
    struct {
        unsigned int bitfield : 4;
    } bf = {0};
    unsigned int *ptr = &bf.bitfield;  /* Taking address of bit-field */
    
    /* Test Case L: __builtin_va_arg misuse in expression */
    va_list ap2;
    /* Using va_arg as part of larger expression */
    int w = __builtin_va_arg(ap2, double) + 3.14f;
    
    return 0;
}

/* Additional function to test error propagation */
void test_error_propagation(void) {
    /* Invalid operation inside sizeof - still needs expansion */
    size_t bad_size = sizeof((void)void_func());
    
    /* Nested comma operators with void */
    int nested = (void_func(), (void)0, 42);
    
    /* Attempt to use __builtin_choose_expr with void type */
    int chosen = __builtin_choose_expr(1, (void)0, 42);
}

/* Force certain optimization passes to be disabled */
#pragma GCC pop_options
