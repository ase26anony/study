/* test_expr_error.c
 * This program contains various constructs designed to trigger the 
 * error_mark_node return path in expr.cc during compilation.
 * The program doesn't need to run successfully - it's meant to fail
 * during the compiler's middle-end expansion phase.
 */

#include <stdarg.h>
#include <stdio.h>

/* Helper functions for testing */
void void_func(void) {}
int int_func(void) { return 0; }

/* Test 1: Invalid operations on void expressions */
void test_void_operations(void) {
    /* Direct assignment from void expression - should fail during expansion */
    int x = (void)void_func();
    
    /* Void expression in conditional context */
    int y = (void_func(), 5);
    
    /* Nested void in complex expression */
    int z = sizeof((void)0) ? 1 : 2;
}

/* Test 2: Misusing __builtin_va_arg */
void test_va_arg_misuse(void) {
    /* Using va_arg without proper va_list initialization */
    va_list ap;
    /* This should fail - ap is not initialized for variadic context */
    float f = __builtin_va_arg(ap, float);
    
    /* Type mismatch in va_arg */
    int x = __builtin_va_arg(ap, double);  /* Wrong type for int argument */
}

/* Variadic function to test va_arg in wrong context */
void variadic_test(int n, ...) {
    va_list ap;
    va_start(ap, n);
    /* Correct usage for comparison */
    int x = va_arg(ap, int);
    va_end(ap);
}

/* Test 3: Malformed compound literals */
void test_compound_literals(void) {
    struct S { int a; int b; };
    
    /* Invalid designator in compound literal */
    struct S *p = &(struct S){ .non_existent = 1 };
    
    /* Taking address of non-lvalue compound literal in invalid context */
    int *q = &(int){42} + 1;  /* Address arithmetic on compound literal */
    
    /* Compound literal with wrong type */
    float *r = &(int){100};  /* Type mismatch */
}

/* Test 4: Target-specific expansion failures */
void test_target_specific(void) {
    /* Vector extensions - may fail on targets without vector support */
    typedef int v4si __attribute__((vector_size(16)));
    v4si a = {1, 2, 3, 4};
    v4si b = {5, 6, 7, 8};
    v4si c = a + b;  /* Vector operation */
    
    /* Overflow builtins with potentially unsupported types */
    int overflow;
    /* Using long double which might not be supported for overflow builtins */
    long double ld1 = 1.0e100L, ld2 = 1.0e100L, ld3;
    int res = __builtin_add_overflow(ld1, ld2, &ld3);
    
    /* Complex types with overflow builtins */
    _Complex double c1 = 1.0 + 2.0i, c2 = 3.0 + 4.0i, c3;
    res = __builtin_add_overflow(c1, c2, &c3);
}

/* Test 5: Transaction Memory constructs (if supported) */
void test_transaction_memory(void) {
    /* Transactional memory - may fail if target doesn't support it */
    int x = 0;
    __transaction_atomic {
        x = 42;
    }
}

/* Test 6: Complex nested invalid operations */
void test_nested_errors(void) {
    /* Invalid void expression inside builtin */
    int x = __builtin_constant_p((void)0);
    
    /* Multiple levels of invalid operations */
    int y = sizeof((void)(printf("test"), 0)) + (void)int_func();
    
    /* Invalid address operations in complex expressions */
    struct BitField { unsigned int bf:4; } bf;
    /* Taking address of bit-field (invalid) in larger expression */
    unsigned int *ptr = (unsigned int*)&bf.bf;
    
    /* Misaligned pointer with __builtin_assume_aligned */
    char buffer[10];
    int *aligned_ptr = __builtin_assume_aligned(buffer + 1, 4);
}

/* Test 7: Invalid operations that might slip through at high optimization */
void test_optimization_sensitive(void) {
    /* This might be optimized away at -O0 but reach expansion at -O3 */
    volatile int *volatile_ptr = (volatile int*)(void)void_func();
    
    /* Invalid cast in dead code that might not be eliminated */
    if (0) {
        int x = (void)((int(*)())void_func)();
    }
    
    /* Complex expression with multiple issues */
    int x = (sizeof((void)0) && (void)int_func()) ? 1 : 0;
}

/* Test 8: Using __builtin_choose_expr with invalid types */
void test_builtin_choose_expr(void) {
    /* __builtin_choose_expr with void type in one branch */
    int x = __builtin_choose_expr(1, (void)0, 42);
    
    /* Type mismatch between branches */
    int y = __builtin_choose_expr(0, 3.14, "string");
}

/* Test 9: Invalid pointer arithmetic */
void test_pointer_arithmetic(void) {
    /* Pointer arithmetic on void pointer */
    void *vp = 0;
    vp = vp + 1;  /* Invalid in standard C, GCC might reject during expansion */
    
    /* Function pointer arithmetic */
    void (*fp)(void) = void_func;
    fp = fp + 1;  /* Invalid */
}

/* Test 10: __builtin_offsetof with invalid arguments */
void test_offsetof_errors(void) {
    /* offsetof with bit-field (invalid) */
    struct BadStruct {
        int normal;
        int bitfield:4;
    };
    
    /* This should fail during expansion */
    size_t off = __builtin_offsetof(struct BadStruct, bitfield);
}

/* Main function just contains all tests */
int main(void) {
    /* Call all test functions to ensure their code is processed */
    test_void_operations();
    test_va_arg_misuse();
    test_compound_literals();
    test_target_specific();
    test_transaction_memory();
    test_nested_errors();
    test_optimization_sensitive();
    test_builtin_choose_expr();
    test_pointer_arithmetic();
    test_offsetof_errors();
    
    return 0;
}
