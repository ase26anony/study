/* test_expr_error.c
 * This program contains various constructs designed to trigger the
 * error_mark_node return path in expr.cc during compilation.
 * The program may not compile successfully - that's the point!
 */

#include <stdarg.h>
#include <stdio.h>

/* Helper functions for various test cases */
void void_func(void) {}
int returns_int(void) { return 42; }

/* Test 1: Invalid operations on void expressions */
void test_void_operations(void) {
    /* Direct assignment of void expression - should fail during expansion */
    int x = (void)void_func();
    
    /* Void expression in conditional context */
    int y = (void_func(), 5);
    
    /* Nested void in complex expression */
    int z = sizeof((void)0) + (void_func(), 10);
}

/* Test 2: Misusing __builtin_va_arg */
void test_va_arg_misuse(void) {
    /* Using va_arg without proper va_list initialization */
    va_list ap;
    /* This should fail - ap is not initialized for variadic arguments */
    int x = __builtin_va_arg(ap, int);
    
    /* Type mismatch in va_arg */
    float f = __builtin_va_arg(ap, float); /* Wrong type for default promotions */
}

/* Variadic function to test va_arg in wrong context */
void variadic_func(int n, ...) {
    va_list ap;
    va_start(ap, n);
    /* Correct usage for comparison */
    int x = va_arg(ap, int);
    /* Incorrect: trying to get struct by value (might fail during expansion) */
    struct S { int a; } s = __builtin_va_arg(ap, struct S);
    va_end(ap);
}

/* Test 3: Malformed compound literals */
void test_compound_literals(void) {
    /* Non-existent field designator */
    struct Point { int x; int y; };
    int *p = &(struct Point){ .z = 5 }; /* .z doesn't exist */
    
    /* Taking address of non-lvalue compound literal in invalid context */
    int *q = &(int){returns_int()};
    
    /* Compound literal with wrong type in pointer context */
    double *dp = &(int){42}; /* Type mismatch */
}

/* Test 4: Target-specific expansion failures */
void test_target_specific(void) {
    /* Vector operations on potentially unsupported targets */
    typedef int v4si __attribute__((vector_size(16)));
    v4si a = {1, 2, 3, 4};
    v4si b = {5, 6, 7, 8};
    v4si c = a + b; /* May fail expansion on non-vector targets */
    
    /* Overflow builtins with complex types */
    _Complex double cd1 = 1.0 + 2.0i;
    _Complex double cd2 = 3.0 + 4.0i;
    /* Using overflow builtin with complex - likely unsupported */
    int overflow = __builtin_add_overflow_p(cd1, cd2, (_Complex double)0);
}

/* Test 5: Transaction Memory constructs without support */
#ifdef __GNUC__
void test_transaction_memory(void) {
    /* Transactional memory - may fail if not supported by target */
    __transaction_atomic {
        int x = 42;
    }
}
#endif

/* Test 6: Complex nested invalid operations */
void test_nested_errors(void) {
    /* Void expression inside __builtin_constant_p */
    int is_const = __builtin_constant_p((void)0);
    
    /* Invalid address operations in complex expressions */
    int bitfield = 0;
    struct BitFieldStruct {
        unsigned int field: 3;
    } bfs = {0};
    
    /* Taking address of bit-field (invalid) */
    unsigned int *ptr = &bfs.field;
    
    /* Misaligned pointer in alignment context */
    char buffer[10];
    int *misaligned = (int*)(buffer + 1);
    int *aligned = __builtin_assume_aligned(misaligned, 4);
}

/* Test 7: Invalid operations in sizeof context */
void test_sizeof_errors(void) {
    /* sizeof on void expression */
    size_t s1 = sizeof((void)returns_int());
    
    /* sizeof on function type (not function pointer) */
    size_t s2 = sizeof(returns_int);
}

/* Test 8: Using statement expressions in invalid ways */
void test_statement_expressions(void) {
    /* Statement expression returning void in value context */
    int x = ({ void_func(); });
    
    /* Nested statement expression with type issues */
    int y = ({ 
        int a = 5;
        void_func();
        a; /* This should be ok, but combined with above might cause issues */
    });
}

/* Main function - just contains all test cases */
int main(void) {
    /* Call test functions to trigger compilation errors */
    test_void_operations();
    test_va_arg_misuse();
    test_compound_literals();
    test_target_specific();
    
#ifdef __GNUC__
    test_transaction_memory();
#endif
    
    test_nested_errors();
    test_sizeof_errors();
    test_statement_expressions();
    
    /* Try to use variadic function incorrectly */
    variadic_func(1, 2, 3);
    
    return 0;
}
