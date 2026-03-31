/* test_expr_error_mark.c
 * This program contains constructs designed to trigger error_mark_node
 * returns during GCC's expression expansion phase (expr.cc).
 */

#include <stdarg.h>
#include <stdio.h>

/* Helper functions */
void void_func(void) {}
int int_func(void) { return 0; }

/* Test 1: Invalid operations on void expressions */
void test_void_operations(void) {
    /* Direct assignment of void expression - should fail during expansion */
    int x = (void)void_func();
    
    /* Void in comma operator in value context */
    int y = (printf("hello"), 5);
    
    /* Void expression in conditional */
    int z = (void_func() ? 1 : 2);
}

/* Test 2: Misusing __builtin_va_arg */
void test_va_arg_misuse(void) {
    /* Using va_arg outside variadic context */
    va_list ap;
    int x = __builtin_va_arg(ap, int);
    
    /* Type mismatch - float vs actual promoted type */
    float f = __builtin_va_arg(ap, float);
}

/* Variadic function to potentially trigger va_arg issues */
void variadic_func(const char *fmt, ...) {
    va_list ap;
    va_start(ap, fmt);
    
    /* Incorrect type in va_arg - might fail during expansion */
    double d = __builtin_va_arg(ap, double);
    
    va_end(ap);
}

/* Test 3: Malformed compound literals */
void test_compound_literals(void) {
    /* Non-existent field designator */
    struct S { int a; int b; };
    int *p = &(struct S){ .non_existent_field = 1 };
    
    /* Compound literal in non-lvalue context with address taken */
    int *q = &(int){42} + 1;
}

/* Test 4: Target-specific expansion failures */
void test_target_specific(void) {
    /* Vector extensions - may fail on targets without vector support */
    typedef int v4si __attribute__((vector_size(16)));
    v4si a = {1, 2, 3, 4};
    v4si b = {5, 6, 7, 8};
    v4si c = a + b;
    
    /* Overflow builtins with potentially unsupported types */
    int overflow;
    long double ld1 = 1.0, ld2 = 2.0, ld_result;
    int overflow_ld = __builtin_add_overflow(ld1, ld2, &ld_result);
}

/* Test 5: Transaction Memory constructs (if supported) */
void test_transaction_memory(void) {
    /* __transaction_atomic might fail expansion if TM not supported */
    __transaction_atomic {
        int x = 42;
    }
}

/* Test 6: Complex nested invalid operations */
void test_complex_nested(void) {
    /* Invalid void expression inside __builtin_constant_p */
    int is_const = __builtin_constant_p((void)0);
    
    /* Nested invalid operations in conditional */
    int x = (void_func(), 0) ? 1 : 2;
    
    /* sizeof with void expression */
    size_t sz = sizeof((void)void_func());
}

/* Test 7: Invalid address operations */
void test_address_operations(void) {
    /* Address of bit-field (if we have a struct with bit-fields) */
    struct BitField {
        unsigned int field:4;
    } bf;
    
    /* Taking address of bit-field - invalid */
    unsigned int *ptr = &bf.field;
    
    /* Address of non-lvalue */
    int *p = &(int_func() + 1);
}

/* Test 8: Misaligned pointer operations */
void test_misaligned(void) {
    char buffer[10];
    int *misaligned = (int*)(buffer + 1);
    
    /* Use misaligned pointer with alignment assumption */
    int *aligned = __builtin_assume_aligned(misaligned, 4);
    *aligned = 42;
}

/* Test 9: Invalid switch case ranges (GCC extension) */
void test_switch_ranges(void) {
    int x = 5;
    switch (x) {
        case 1 ... 10:  /* Valid case range */
            break;
        case 20 ... 15: /* Invalid reversed range - might fail during expansion */
            break;
    }
}

/* Test 10: Using __builtin_choose_expr with invalid types */
void test_builtin_choose(void) {
    /* Second and third operands with incompatible types */
    int result = __builtin_choose_expr(1, (void)0, 42);
}

/* Main function - container for all tests */
int main(void) {
    /* Call various test functions to trigger different error conditions */
    test_void_operations();
    test_va_arg_misuse();
    test_compound_literals();
    test_target_specific();
    test_transaction_memory();
    test_complex_nested();
    test_address_operations();
    test_misaligned();
    test_switch_ranges();
    test_builtin_choose();
    
    /* Also test some inline expressions */
    variadic_func("test %d %f", 42, 3.14);
    
    return 0;
}
