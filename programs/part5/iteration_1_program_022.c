/* test_expr_error_mark.c
 * This program contains various constructs designed to trigger
 * error_mark_node returns during expression expansion in GCC's expr.cc
 */

#include <stdarg.h>
#include <stdio.h>

/* Helper functions */
void void_func(void) {}
int int_func(void) { return 42; }

/* Test 1: Invalid operations on void expressions */
void test_void_operations(void) {
    /* Direct void assignment - should fail during expansion */
    int x = (void)void_func();
    
    /* Void in comma operator in value context */
    int y = (printf("hello"), 5);
    
    /* Void in conditional expression */
    int z = (1 ? (void)0 : 0);
}

/* Test 2: Misusing __builtin_va_arg */
void test_va_arg_misuse(void) {
    /* Using va_arg without proper va_list initialization */
    int x = __builtin_va_arg(ap, int);
    
    /* Type mismatch in va_arg */
    va_list args;
    int y = __builtin_va_arg(args, float);  /* float vs int promotion issues */
}

/* Test 3: Malformed compound literals */
void test_compound_literals(void) {
    /* Non-existent field designator */
    struct S { int a; int b; };
    int *p = &(struct S){ .non_existent = 1 };
    
    /* Taking address in invalid context */
    int *q = &(int){1} + 1;  /* Compound literal might not be an lvalue here */
}

/* Test 4: Target-specific expansion failures */
void test_target_specific(void) {
    /* Vector operations on potentially unsupported targets */
    typedef int v4si __attribute__((vector_size(16)));
    v4si a = {1, 2, 3, 4};
    v4si b = {5, 6, 7, 8};
    v4si c = a + b;  /* May fail expansion on non-vector targets */
    
    /* Overflow builtins with complex types */
    _Complex double cd1 = 1.0 + 2.0i;
    _Complex double cd2 = 3.0 + 4.0i;
    _Complex double cd3;
    int overflow = __builtin_add_overflow(cd1, cd2, &cd3);
}

/* Test 5: Transaction Memory without support */
#ifdef __GNUC__
void test_transaction_memory(void) {
    __transaction_atomic {
        int x = 42;
    }
}
#endif

/* Test 6: Complex nested invalid operations */
void test_nested_errors(void) {
    /* Invalid void expression inside builtin */
    int x = __builtin_constant_p((void)0);
    
    /* Multiple levels of invalid operations */
    int y = sizeof((void)void_func());
    
    /* Invalid address of bit-field in struct */
    struct BitField {
        unsigned int field:4;
    } bf;
    unsigned int *ptr = &bf.field;  /* Taking address of bit-field */
    
    /* Misaligned pointer with __builtin_assume_aligned */
    char buffer[10];
    int *misaligned = (int*)(buffer + 1);
    int *aligned = __builtin_assume_aligned(misaligned, 4);
}

/* Test 7: Invalid operations in different optimization contexts */
void test_optimization_sensitive(void) {
    /* This might be optimized away at -O2 but reach expansion at -O0 */
    volatile int *volatile_ptr = (volatile int*)0xDEADBEEF;
    int value = *volatile_ptr + ((void)void_func(), 0);
}

/* Test 8: Using __builtin_choose_expr with invalid types */
void test_builtin_choose(void) {
    /* Second and third operands have incompatible types */
    int x = __builtin_choose_expr(1, (void)0, 42);
}

/* Test 9: Invalid pointer arithmetic */
void test_pointer_arithmetic(void) {
    /* Pointer to void arithmetic */
    void *vp = 0;
    void *vp2 = vp + 1;  /* GCC extension but might fail in some contexts */
    
    /* Function pointer arithmetic */
    void (*fp)(void) = void_func;
    fp = fp + 1;  /* Invalid function pointer arithmetic */
}

/* Test 10: Using statement expressions in invalid contexts */
void test_statement_expressions(void) {
    /* Statement expression returning void in value context */
    int x = ({ void_func(); });
    
    /* Nested statement expressions with type mismatches */
    int y = ({ 
        int a = 5;
        ({ void_func(); a; });  /* Inner returns void? */
    });
}

/* Main function - just contains all test calls */
int main(void) {
    /* These calls don't need to execute - we just need them to compile
     * (or fail during compilation at the right phase) */
    test_void_operations();
    test_va_arg_misuse();
    test_compound_literals();
    test_target_specific();
    
#ifdef __GNUC__
    test_transaction_memory();
#endif
    
    test_nested_errors();
    test_optimization_sensitive();
    test_builtin_choose();
    test_pointer_arithmetic();
    test_statement_expressions();
    
    return 0;
}
