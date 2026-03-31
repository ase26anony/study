/* test_expr_error.c
 * This program contains various constructs designed to trigger the
 * error_mark_node return path in expr.cc during compilation.
 * The program may not compile successfully - that's the point!
 */

#include <stdarg.h>
#include <stdio.h>

/* Helper functions */
void void_func(void) { }
int variadic_func(int count, ...) { return 0; }

/* Vector type that might fail expansion on some targets */
typedef int v4si __attribute__((vector_size(16)));

/* Test 1: Invalid operations on void expressions */
void test_void_errors(void) {
    /* Direct void assignment - should fail during expansion */
    int x = (void)void_func();
    
    /* Void in comma operator in value context */
    int y = (printf("hello"), 5);
    
    /* Void expression in conditional */
    int z = (void_func() ? 1 : 0);
    
    /* Taking address of void expression */
    void *ptr = &(void)void_func();
}

/* Test 2: Misusing __builtin_va_arg */
void test_va_arg_errors(void) {
    /* Using va_arg without proper va_list initialization */
    va_list ap;
    int x = __builtin_va_arg(ap, int);
    
    /* Type mismatch in va_arg */
    int y = __builtin_va_arg(ap, float);  /* float vs int promotion */
    
    /* Using va_arg on non-va_list */
    int *p;
    int z = __builtin_va_arg(p, int);
}

/* Test 3: Malformed compound literals */
void test_compound_literal_errors(void) {
    struct S { int a; int b; };
    
    /* Non-existent field designator */
    struct S s1 = (struct S){ .non_existent = 1 };
    
    /* Compound literal in invalid context */
    int *p = &(int){ .x = 1 };  /* int has no member x */
    
    /* Address of non-lvalue compound literal in complex expression */
    int **pp = &(&(int){42});
}

/* Test 4: Target-specific expansion failures */
void test_target_specific_errors(void) {
    /* Vector operations - may fail on targets without vector support */
    v4si a = {1, 2, 3, 4};
    v4si b = {5, 6, 7, 8};
    v4si c = a + b;
    
    /* Overflow builtins with potentially unsupported types */
    long double ld1 = 1.0e100L, ld2 = 2.0e100L;
    int overflow;
    __builtin_add_overflow(ld1, ld2, &overflow);
    
    /* Complex type in overflow builtin */
    _Complex double cd1 = 1.0 + 2.0i, cd2 = 3.0 + 4.0i;
    __builtin_add_overflow(cd1, cd2, &overflow);
}

/* Test 5: Transaction Memory constructs (if supported) */
void test_tm_errors(void) {
    /* Transactional memory - may fail if TM not supported */
    __transaction_atomic {
        int x = 42;
    }
    
    /* Nested TM with invalid operations */
    __transaction_atomic {
        int y = (void)void_func();
    }
}

/* Test 6: Complex nested invalid operations */
void test_complex_nested_errors(void) {
    /* Invalid void expression inside __builtin_constant_p */
    int x = __builtin_constant_p((void)0);
    
    /* Invalid address operation in sizeof */
    int y = sizeof(&(void)void_func());
    
    /* Bit-field address attempt */
    struct BitField { unsigned int bf:4; } bf;
    unsigned int *ptr = &bf.bf;  /* Cannot take address of bit-field */
    
    /* Misaligned pointer with __builtin_assume_aligned */
    char buffer[10];
    int *aligned_ptr = __builtin_assume_aligned(buffer + 1, 4);
}

/* Test 7: Invalid operations in different optimization contexts */
void test_optimization_sensitive_errors(void) {
    /* This might be folded away at -O2 but reach expansion at -O0 */
    int x = (*(int*)0) + 5;  /* Dereference null pointer */
    
    /* Division by zero that might be caught at different phases */
    int y = 1 / 0;
    
    /* Invalid shift that might pass early folding */
    int z = 1 << -1;
}

/* Test 8: Variadic function misuse */
void test_variadic_errors(void) {
    /* Wrong argument types to variadic function */
    variadic_func(3, "string", 1.5, void_func());
    
    /* Empty variadic call with wrong expectation */
    variadic_func(0);  /* But function might expect at least one arg */
}

/* Test 9: __builtin_choose_expr with invalid types */
void test_builtin_choose_errors(void) {
    /* Choose expression with incompatible types */
    int x = __builtin_choose_expr(1, (void)0, 42);
    
    /* Choose expression where both arms are invalid */
    int y = __builtin_choose_expr(0, &(void)void_func(), (void)0);
}

/* Test 10: Offsetof with invalid members */
#include <stddef.h>
void test_offsetof_errors(void) {
    struct Invalid {
        int x;
        void func(void);
    };
    
    /* Offsetof with function member */
    size_t off = offsetof(struct Invalid, func);
    
    /* Offsetof with non-existent member */
    size_t off2 = offsetof(struct Invalid, non_existent);
}

int main(void) {
    /* We don't actually call these functions at runtime.
     * Their mere presence with invalid constructs should trigger
     * compilation errors during expression expansion. */
    
    test_void_errors();
    test_va_arg_errors();
    test_compound_literal_errors();
    test_target_specific_errors();
    test_tm_errors();
    test_complex_nested_errors();
    test_optimization_sensitive_errors();
    test_variadic_errors();
    test_builtin_choose_errors();
    test_offsetof_errors();
    
    return 0;
}
