/* test_expr_error_mark.c
 * This program contains constructs designed to trigger error_mark_node
 * returns in expr.cc during the RTL expansion phase.
 */

#include <stdarg.h>
#include <stdio.h>

/* Helper functions */
void void_func(void) { }
int int_func(void) { return 42; }

/* Test 1: Invalid operations on void expressions */
void test_void_operations(void) {
    /* Direct assignment from void expression */
    int x = (void)void_func();  /* Should trigger error during expansion */
    
    /* Void expression in comma operator */
    int y = (printf("test"), 5);  /* Left side is void */
    
    /* Void in conditional expression */
    int z = (void_func() ? 1 : 2);  /* Condition is void */
}

/* Test 2: Misusing __builtin_va_arg */
void test_va_arg_misuse(void) {
    /* Using va_arg without proper va_list initialization */
    va_list ap;
    /* This should fail during expansion as ap is not initialized in a variadic context */
    int x = __builtin_va_arg(ap, int);
    
    /* Type mismatch in va_arg */
    float f = __builtin_va_arg(ap, float);  /* Wrong type promotion */
}

/* Variadic function to create proper context */
void variadic_func(const char *fmt, ...) {
    va_list ap;
    va_start(ap, fmt);
    /* Still misuse by using wrong type */
    double d = __builtin_va_arg(ap, double);  /* Might fail if passed int */
    va_end(ap);
}

/* Test 3: Malformed compound literals */
void test_compound_literals(void) {
    struct S { int a; int b; };
    
    /* Invalid designator */
    int *p1 = &(int){ .non_existent = 1 };  /* Non-existent field */
    
    /* Taking address in invalid context */
    int *p2 = &(int){1} + 1;  /* Address of temporary in arithmetic */
    
    /* Type mismatch in compound literal */
    struct S *p3 = &(struct S){ .a = 1, .b = 2.5 };  /* Type conversion issue */
}

/* Test 4: Target-specific expansion failures */
void test_target_specific(void) {
    /* Vector extensions on potentially unsupported targets */
    typedef int v4si __attribute__((vector_size(16)));
    v4si a = {1, 2, 3, 4};
    v4si b = {5, 6, 7, 8};
    v4si c = a + b;  /* May fail expansion on non-vector targets */
    
    /* Overflow builtins with complex types */
    _Complex double cd1 = 1.0 + 2.0i;
    _Complex double cd2 = 3.0 + 4.0i;
    /* __builtin_add_overflow_p(cd1, cd2, cd1); */ /* Uncomment if supported */
    
    /* Transactional memory without proper support */
    #ifdef __GNUC__
    /* __transaction_atomic { */
    /*     int x = 5; */
    /* } */
    #endif
}

/* Test 5: Complex nested invalid operations */
void test_nested_errors(void) {
    /* Invalid void expression inside sizeof */
    size_t s1 = sizeof((void)void_func());
    
    /* Invalid void in __builtin_constant_p */
    int b1 = __builtin_constant_p((void)0);
    
    /* Complex conditional with void */
    int x = (void_func(), 0) ? int_func() : (void)void_func();
    
    /* Nested address operations */
    int bitfield = 7;
    /* int *ptr = &(bitfield & 3); */  /* Address of bitwise operation result */
}

/* Test 6: Alignment issues */
void test_alignment(void) {
    char buffer[10];
    int *misaligned = (int*)(buffer + 1);  /* Potentially misaligned */
    
    /* Force alignment assumption on misaligned pointer */
    int *aligned = __builtin_assume_aligned(misaligned, 4);
    *aligned = 42;  /* May trigger error during expansion */
}

/* Test 7: Invalid operations in different optimization contexts */
void test_optimization_sensitive(void) {
    /* This might be optimized away at -O2 but reach expansion at -O0 */
    volatile int *volatile_ptr = 0;
    int value = *volatile_ptr + (void)void_func();
    
    /* Complex expression with side effects */
    int x = (void_func(), ({ void_func(); int_func(); }), 0);
}

/* Main function - container for all tests */
int main(void) {
    /* Call test functions to trigger compilation errors */
    test_void_operations();
    test_va_arg_misuse();
    test_compound_literals();
    test_target_specific();
    test_nested_errors();
    test_alignment();
    test_optimization_sensitive();
    
    /* Call variadic function with wrong argument type */
    variadic_func("%d", 42);  /* Passing int when double expected */
    
    return 0;
}
