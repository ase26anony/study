/* test_expr_error.c
 * This program contains various constructs designed to trigger the
 * error_mark_node return path in expr.cc during compilation.
 * The program may not compile successfully - that's the point.
 */

#include <stdarg.h>
#include <stdio.h>

/* Helper functions for various test cases */
void void_func(void) {}
int returns_int(void) { return 42; }

/* Test 1: Invalid operations on void expressions */
void test_void_operations(void) {
    /* Direct assignment from void expression - should fail during expansion */
    int x = (void)void_func();
    
    /* Void expression in arithmetic context */
    int y = 5 + (void)void_func();
    
    /* Void in conditional operator */
    int z = (void)void_func() ? 1 : 0;
    
    /* Nested void in sizeof - might pass parsing but fail during expansion */
    size_t s = sizeof((void)void_func());
}

/* Test 2: Misusing __builtin_va_arg in invalid contexts */
void test_va_arg_misuse(void) {
    va_list ap;
    
    /* Using va_arg outside variadic function context */
    int x = __builtin_va_arg(ap, int);
    
    /* Type mismatch with promoted types */
    float f = __builtin_va_arg(ap, float);  /* float promotes to double in varargs */
    
    /* Using with completely wrong type */
    struct S { int a; } s = __builtin_va_arg(ap, struct S);
}

/* Test 3: Malformed compound literals */
void test_compound_literals(void) {
    struct Point { int x; int y; };
    
    /* Invalid designator */
    struct Point *p1 = &(struct Point){ .z = 5 };  /* .z doesn't exist */
    
    /* Taking address in invalid context */
    int *addr = &(int){10} + 1;  /* Address arithmetic on compound literal */
    
    /* Compound literal with wrong type in pointer context */
    double *dptr = (double*)&(int){42};
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
    int overflow;
    
    /* This may fail if backend doesn't support complex overflow checking */
    __builtin_add_overflow(cd1, cd2, &cd1);
}

/* Test 5: Transactional Memory constructs without proper support */
#ifdef __GNUC__
void test_transactional_memory(void) {
    int x = 0;
    
    /* Transactional memory block - may fail expansion if TM not supported */
    __transaction_atomic {
        x = 42;
    }
    
    /* Nested transaction with void expression */
    __transaction_atomic {
        (void)void_func();
    }
}
#endif

/* Test 6: Complex nested invalid expressions */
void test_complex_nested(void) {
    /* Invalid address operations in complex expressions */
    int bitfield : 4;
    
    /* Taking address of bitfield in sizeof context */
    size_t sz = sizeof(&bitfield);
    
    /* Nested invalid operations */
    int x = __builtin_constant_p((void)0) ? 1 : 0;
    
    /* Misaligned pointer in alignment context */
    char buffer[10];
    int *misaligned = (int*)(buffer + 1);
    int *aligned = __builtin_assume_aligned(misaligned, 4);
}

/* Test 7: Using __builtin_choose_expr with invalid types */
void test_builtin_choose(void) {
    /* Second and third operands must have compatible types */
    int x = __builtin_choose_expr(1, (void)0, 42);
    
    /* With __builtin_types_compatible_p and invalid expression */
    int y = __builtin_choose_expr(
        __builtin_types_compatible_p(int, void),
        (void)void_func(),
        42
    );
}

/* Test 8: Invalid pointer conversions in constant expressions */
void test_pointer_conversions(void) {
    /* Converting integer to function pointer */
    void (*func_ptr)(void) = (void (*)(void))0x1000;
    
    /* In constant expression context */
    static int *ptr = (int*)((void)0);
    
    /* Pointer to void in non-pointer context */
    int addr = (int)(void*)&returns_int;
}

/* Test 9: Using statement expressions with void */
void test_statement_expressions(void) {
    /* Statement expression returning void in value context */
    int x = ({ void_func(); });
    
    /* Nested statement expressions with type mismatch */
    int y = ({ 
        if (1) 
            void_func(); 
        else 
            returns_int(); 
    });
}

/* Test 10: __builtin_constant_p with obviously non-constant invalid expressions */
void test_constant_p(void) {
    /* These should be evaluable at compile time but are invalid */
    int x = __builtin_constant_p((void)returns_int());
    
    /* With va_arg */
    va_list ap;
    int y = __builtin_constant_p(__builtin_va_arg(ap, int));
}

/* Main function - just calls all tests */
int main(void) {
    /* Don't actually call these at runtime - they're for compile-time testing */
    /* The compiler should fail during expansion before generating executable code */
    
    test_void_operations();
    test_va_arg_misuse();
    test_compound_literals();
    test_target_specific();
    
#ifdef __GNUC__
    test_transactional_memory();
#endif
    
    test_complex_nested();
    test_builtin_choose();
    test_pointer_conversions();
    test_statement_expressions();
    test_constant_p();
    
    return 0;
}
